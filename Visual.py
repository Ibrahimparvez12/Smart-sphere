from vpython import *
import pandas as pd

# Read data using pandas
data = pd.read_csv('Data15.csv')

# Create a 3D scene
scene = canvas(title="IMU Data Visualization", width=1920, height=1080)

# Create 3D axes
x_axis = arrow(pos=vector(0, 0, 0), axis=vector(1, 0, 0), color=color.red)
y_axis = arrow(pos=vector(0, 0, 0), axis=vector(0, 1, 0), color=color.green)
z_axis = arrow(pos=vector(0, 0, 0), axis=vector(0, 0, 1), color=color.blue)

# Scale factor for position and orientation
position_scale = 50
orientation_scale = 0.3

# Plot IMU data
while True:
    for index, row in data.iterrows():
        x, y, z = row['Px']* position_scale, row['Py'] * position_scale, row['Pz'] * position_scale
        #ex, ey, ez = row['Ex'], row['Ey'], row['Ez']
        sphere(pos=vector(x, y, z), radius=0.1, color=color.white)

        '''# Convert orientation to a vector
        ex, ey, ez = row['Ex'], row['Ey'], row['Ez']
        orientation_vector = vector(ex, ey, ez)
        orientation_vector = norm(orientation_vector) * orientation_scale
        
        arrow(pos=vector(x, y, z), axis=orientation_vector, color=color.red)
        arrow(pos=vector(x, y, z), axis=orientation_vector, color=color.green)
        arrow(pos=vector(x, y, z), axis=orientation_vector, color=color.blue)'''
        rate(60)
