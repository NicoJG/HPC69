include("$hpc_folder/common/check_submission.jl")


struct CheckCaseCellDistances <: CheckCase
  cells_path      :: String
  num_threads     :: Int
  reference_paths :: Union{Nothing,Vector{String}}
  hyperfine       :: HyperfineSetup
end

function CheckCaseCellDistances(
    cells_suffix  :: String,
    use_reference :: Bool,
    hyperfine     :: HyperfineSetup
  )
  @assert hascore_limit(hyperfine)
  basepath = "$hpc_folder/cell_distances/test_data"
  if use_reference
    requires_stdout!(hyperfine)
  end
  CheckCaseCellDistances(
    "$basepath/cells_$cells_suffix",
    core_limit(hyperfine),
    !use_reference ? nothing :
      ["$basepath/dist_$(cells_suffix)_trunc",
       "$basepath/dist_$(cells_suffix)_trunc_shift",
       "$basepath/dist_$(cells_suffix)_round"],
    hyperfine
    )
end


function Base.show(io::IO, case::CheckCaseCellDistances)
  cell_filename = basename(case.cells_path)
  cell_suffix = cell_filename[findlast('_',cell_filename)+1:end]
  show(io, "$(cell_suffix) with $(case.num_threads) threads")
end

hyperfine_setup(case::CheckCaseCellDistances) = case.hyperfine

build_exec_name(::Type{CheckCaseCellDistances}) = "cell_distances"

function command(case::CheckCaseCellDistances) :: Cmd
  symlink_path = "./cells"
  if ispath(symlink_path)
    rm(symlink_path)
  end
  symlink(case.cells_path, symlink_path)
  return command(hyperfine_setup(case), `./cell_distances -t$(case.num_threads)`)
end

function verify(
    case::CheckCaseCellDistances,
    stdout_log::String
  ) :: Union{Nothing,ErrorException}
  tol_abs = 50
  tol_rel = 0.02
  isnothing(case.reference_paths) && return

  stdout_dist = parse_cell_distances(stdout_log)
  if stdout_dist isa ErrorException
    err_msg = stdout_dist.msg
    @goto stdout_dump
  end
  stdout_keys = collect(keys(stdout_dist))

  ref_errors = Vector{ErrorException}()
  for ref_path in case.reference_paths
    ref_dist = parse_cell_distances(read(ref_path, String))
    if ref_dist isa ErrorException
      throw(ref_dist)
    end
    ref_keys = collect(keys(ref_dist))

    for k in setdiff(stdout_keys, ref_keys)
      if stdout_dist[k] > tol_abs
        push!(ref_errors,
          ErrorException(
            "Error when comparing against reference solution $(basename(ref_path)):\n" *
            "value at $(k/100) should be zero but is $(stdout_dist[k]) " *
            "(applying absolute error tolerance of $tol_abs)"))
        @goto verify__comparison_done
      end
    end

    for k in setdiff(ref_keys, stdout_keys)
      if ref_dist[k] > tol_abs
        push!(ref_errors,
          ErrorException(
            "Error when comparing against reference solution $(basename(ref_path)):\n" *
            "value at $(k/100) should be $(ref_dist[k]) but is zero " *
            "(applying absolute error tolerance of $tol_abs)"))
        @goto verify__comparison_done
      end
    end

    for k in intersect(stdout_keys, ref_keys)
      tol = floor(UInt, max(tol_abs, tol_rel*ref_dist[k]))
      count_min = tol < ref_dist[k] ? ref_dist[k] - tol : UInt(0)
      count_max = ref_dist[k] + tol
      if stdout_dist[k] < count_min || stdout_dist[k] > count_max
        push!(ref_errors,
          ErrorException(
            "Error when comparing against reference solution $(basename(ref_path)):\n" *
            "value at $(k/100) should be between $count_min and $count_max " *
            "but is $(stdout_dist[k]) " *
            "(applying absolute error tolerance of $tol_abs and " *
            "relative error tolerance $tol_rel " *
            "to reference value $(ref_dist[k]))"))
        @goto verify__comparison_done
      end
    end

    return
    @label verify__comparison_done 
  end
  err_msg = join((e.msg for e in ref_errors), "\n")

  @label stdout_dump
  stdout_dump_path = "distances_stdout"
  write(stdout_dump_path, stdout_log)
  return ErrorException(
    "Output of program was written to $stdout_dump_path.\n" *
    err_msg)
end

function parse_cell_distances(str::String) :: Union{Dict{Int,UInt},ErrorException}
  dist_dict = Dict{Int,UInt}()

  if isempty(str)
    return ErrorException("Error when parsing: string is empty")
  end
  rows = split(str, "\n")

  for (i,r) in enumerate(rows)
    all(isspace, r) && continue

    d = ""; c = ""
    try
      (d,c) = split(r, " ") 
    catch e
      return ErrorException("Error when parsing: " *
        "$i-th row $r cannot be split into exactly two parts")
    end

    if length(d) != 5
      return ErrorException("Error when parsing: " *
        "$i-th distance $d not of required string length.")
    end
    if d[3] != '.'
      return ErrorException("Error when parsing: " *
        "middle character of $i-th distance $d is not a point.")
    end
    if any(!isdigit, [d[1],d[2],d[4],d[5]])
      return ErrorException("Error when parsing: " *
        "$i-th distance $d does not contain digit where it should.")
    end
    dparsed = (parse(Int,d[1])*1000 + parse(Int,d[2])*100 +
               parse(Int,d[4])*10 + parse(Int,d[5]))

    cparsed = UInt(0)
    try
      cparsed = parse(UInt,c)
    catch e
      return ErrorException("Error when parsing: " *
        "$i-th count $c is not parsable as unsigned integer.")
    end

    if cparsed != 0
      dist_dict[dparsed] = cparsed
    end
  end

  return dist_dict
end

