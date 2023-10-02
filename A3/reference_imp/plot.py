#%% 
import matplotlib.pyplot as plt

# Load the PPM image using matplotlib
files = ["newton_attractors_x5.ppm", "newton_convergence_x5.ppm"]
file_names = ["newton_attractors.png", "newton_convergence.png"]

for file, name in zip(files, file_names):
    image = plt.imread(file)
    plt.imshow(image)   
    plt.show()
    plt.savefig(name)