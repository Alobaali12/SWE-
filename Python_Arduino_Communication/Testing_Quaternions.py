import numpy as np
from mpl_toolkits.mplot3d import Axes3D
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
from scipy.spatial.transform import Rotation

# Function to update the plot
def update_plot(frame):
    global ax, r
    ax.cla()  # Clear the current plot

    # Generate a rotation matrix from the quaternion
    quaternion = imu_data[frame]  # Replace imu_data with your actual quaternion data
    rotation = Rotation.from_quat(quaternion)
    rot_matrix = rotation.as_matrix()

    # Generate a set of points forming a cube
    points = np.array([[1, 1, 1], [-1, 1, 1], [-1, -1, 1], [1, -1, 1],
                      [1, 1, -1], [-1, 1, -1], [-1, -1, -1], [1, -1, -1]])

    # Rotate the points using the rotation matrix
    rotated_points = np.dot(points, rot_matrix.T)

    # Plot the rotated cube
    ax.scatter(rotated_points[:, 0], rotated_points[:, 1], rotated_points[:, 2])
    ax.set_xlim([-2, 2])
    ax.set_ylim([-2, 2])
    ax.set_zlim([-2, 2])

# Generate example quaternion data (replace with your actual data)
imu_data = np.random.rand(100, 4)  # Shape: (num_samples, 4)

# Create the figure and 3D axes
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')

# Create the animation
animation = FuncAnimation(fig, update_plot, frames=range(imu_data.shape[0]), interval=50)

# Show the plot
plt.show()





