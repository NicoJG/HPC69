# create the wanted output for a given cell data file
import numpy as np 
from scipy.spatial import distance_matrix

directory = "data/"
file_name = "cells_1e4"

input_file = directory+file_name
output_file = directory+"python_output_"+file_name

coords = np.genfromtxt(directory+file_name, unpack=True)

dist_mat = distance_matrix(coords.T, coords.T)
# we don't want the self distance and each pair only once
# only take the upper triangular matrix (without diagonal)
mask = np.zeros_like(dist_mat, dtype=bool)
mask[np.tril_indices_from(mask)] = True
dist_mat[mask] = np.nan
dist_mat = np.round(dist_mat, 2)

print(dist_mat)

max_dist = 34.64
possible_dists = np.arange(0, max_dist+0.01, 0.01)
bin_edges = possible_dists - 0.005
bin_edges = np.append(bin_edges, max_dist + 0.005)
counts, bin_edges = np.histogram(dist_mat, bins=bin_edges)

output_data = list(zip(possible_dists, counts))
np.savetxt(output_file, output_data, fmt='%05.2f %i')

with open(output_file, "a") as f:
    f.write(f"Counted distances: {np.sum(counts)}\n")
