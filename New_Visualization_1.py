import pandas as pd
import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import seaborn as sns
import numpy as np

# Load CSV file
file_path = "imu_data_localized.csv"
df = pd.read_csv(file_path)

# Extract data for plotting
x_data = df['Px']
y_data = df['Py']
z_data = df['Pz']

# 3D Trajectory Plot
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
ax.plot(x_data, y_data, z_data, label='3D Trajectory', color='blue')
ax.set_xlabel('X Position')
ax.set_ylabel('Y Position')
ax.set_zlabel('Z Position')
ax.set_title('3D Trajectory of the Ball')
plt.show()

# 2D Projection Plot (XY Plane)
plt.figure(figsize=(8, 6))
plt.plot(x_data, y_data, label='Trajectory in XY Plane', color='red')
plt.xlabel('X Position')
plt.ylabel('Y Position')
plt.title('2D Projection of Ball Trajectory (XY Plane)')
plt.grid(True)
plt.legend()
plt.show()

plt.figure(figsize=(8, 6))
# Use keyword arguments for x and y axes
sns.kdeplot(x=x_data, y=y_data, cmap="Blues", fill=True)
plt.xlabel('X Position')
plt.ylabel('Y Position')
plt.title('Density of Ball Positions in XY Plane')
plt.grid(True)
plt.show()
