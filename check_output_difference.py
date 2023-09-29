import numpy as np

our_dists, our_counts = np.genfromtxt("output.txt", unpack=True, skip_header=104, skip_footer=2)
target_dists, target_counts = np.genfromtxt("data/dist_1e5_short_round", unpack=True)

diffs = our_counts - target_counts[:-1]

print(f"Total counts (our): {np.sum(our_counts)}")
print(f"Total counts (target): {np.sum(target_counts)}")
print(f"Maximum difference: {np.abs(diffs).max()}")
