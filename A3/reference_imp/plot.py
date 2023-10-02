#%% 
import matplotlib.pyplot as plt
import matplotlib.image as mpim

# Load the PPM image using matplotlib
image = mpim.imread("newton_convergence_x5.ppm")
print(image.shape)
plt.imshow(image)

# Show the image in a window
plt.show()
# plt.savefig("fig2.png")
