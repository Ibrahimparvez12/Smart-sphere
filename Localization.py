#CSV FILE NAME

import filterpy
import numpy as np
import pandas as pd
from filterpy.kalman import MerweScaledSigmaPoints
from filterpy.kalman import UnscentedKalmanFilter as UKF
import plotly.graph_objs as go
import csv
import time

class CustomSigmaPoints(MerweScaledSigmaPoints):
    def __init__(self, n, alpha, beta, kappa, dim_x):
        super().__init__(n, alpha, beta, kappa, dim_x)
        self.dim_x = dim_x  # Initialize dim_x attribute
        self._alpha = alpha
        self._beta = beta
        self._kappa = kappa

    def sigma_points(self, x, P):
        # Generate sigma points based on the Unscented Transformation
        # Custom implementation based on the Unscented Transformation
        # Calculate sigma points using the Cholesky decomposition of P
        # Here, we'll use the standard Unscented Transformation approach

        # Number of sigma points
        num_sigmas = 2 * self.dim_x + 1

        # Compute sigma point scaling factor
        sqrt_n = np.sqrt(self.dim_x)
        lambda_ = self._alpha**2 * (self.dim_x + self._kappa) - self.dim_x
        sigma_scale = np.sqrt(self.dim_x + lambda_)

        # Cholesky decomposition of covariance matrix P
        sqrt_P = np.linalg.cholesky(P)

        # Generate sigma points
        sigma_points = np.zeros((num_sigmas, self.dim_x))
        sigma_points[0] = x
        for i in range(self.dim_x):
            sigma_points[i+1] = x + sigma_scale * sqrt_P[i]
            sigma_points[self.dim_x+i+1] = x - sigma_scale * sqrt_P[i]

        return sigma_points

    def weights(self):
        # Calculate the weights associated with the sigma points
        # Custom implementation based on the Unscented Transformation
        # Here, we'll use the standard weights defined by the UT method
        mean = 1 / (2 * (self.dim_x + self._kappa))
        Wc = np.full(2 * self.dim_x + 1, mean)
        Wm = np.full(2 * self.dim_x + 1, mean)
        Wc[0] = self._kappa / (self.dim_x + self._kappa) + (1 - self._alpha ** 2 + self._beta)
        Wm[0] = self._kappa / (self.dim_x + self._kappa)

        return Wm, Wc

# Create CustomSigmaPoints object
custom_points = CustomSigmaPoints(n=6, alpha=0.1, beta=2.0, kappa=1.0, dim_x=6)

#dt = imu_data['Time'].diff().fillna(0.01) #time difference between consecutive data points
position_global = []
orientation_global = []
covariance_global = []

def read_csv(filename):
    data = []
    with open(filename, 'r') as file:
        reader = csv.reader(file)
        next(reader)  # Skip the header row
        for row in reader:
            data.append([float(value) for value in row])
    return data

def process_data(data):
    for row in data:
        x1 = row[28:31]
        x2 = row[6:9]
        x = x1 + x2
        velocity = row[25:28]
        angular_velocity = row[15:18]
        dt = 0.09 #time difference between consecutive data points

        # Assuming the remaining columns contain other data

        # Call the function to pass velocity and angular_velocity to the UKF predict function

        # State transition function
        def state_transition(x, dt, velocity, angular_velocity):
          """
          Define the state transition function.

          Args:
          - x: Current state vector [x, y, z, roll, pitch, yaw]
          - dt: Time step
          - velocity: Current velocity vector [vx, vy, vz]
          - angular_velocity: Current angular velocity vector [wx, wy, wz]

          Returns:
          - x_next: Predicted next state vector
          """
          # Compute position update
          position_update = np.array(velocity) * dt

          # Compute orientation update
          orientation_update = np.array(angular_velocity) * dt

          # Combine position and orientation updates
          state_update = np.concatenate((position_update, orientation_update))

          # Update the current state to get the predicted next state
          x_next = x + state_update

          return x_next

        # Measurement function
        def measurement_function(x):
            # x: state vector [x, y, z, roll, pitch, yaw]
            # Implement measurement function
            return x

        # UKF Process Model
        def process_model(x, dt, velocity, angular_velocity):
            # Implement the process model
            return state_transition(x, dt, velocity, angular_velocity)

        # UKF Measurement Model
        def measurement_model(x):
            # Implement the measurement model
            return measurement_function(x)

        # Define process noise covariance matrix (Q) and measurement noise covariance matrix (R)
        Q = np.eye(6)  # Assuming process noise is identity
        R = np.eye(6)  # Assuming measurement noise is identity

        # Initial state and covariance
        x0 = np.array([0, 0, 0, 0, 0, 0])  # Initial state vector
        P0 = np.eye(6)  # Initial state covariance matrix


        # Create UKF object
        ukf = UKF(dim_x=6, dim_z=6, fx=process_model, hx=measurement_model, dt=0.09, points=custom_points)

        # Initialize UKF
        ukf.x = x0
        ukf.P = P0
        ukf.Q = Q
        ukf.R = R

        # Run UKF prediction and update steps
        z = np.array(x)  # Sensor measurement

        # Define sample values for velocity and angular velocity
        #velocity = [1.0, 0.0, 0.0]  # Assuming constant velocity along the x-axis
        #angular_velocity = [0.0, 0.0, 0.1]  # Assuming constant angular velocity around the z-axis

        # ukf.predict(velocity=velocity, angular_velocity=angular_velocity)
        ukf.predict(velocity=velocity, angular_velocity=angular_velocity)
        ukf.update(z)

        # Testing
        #print(position_global)
        position_global.append(list(ukf.x[:3]))
        orientation_global.append(list(ukf.x[3:]))
        covariance_global.append(list(ukf.P[:3, :3]))

    return 0

# Specify the path to CSV file
filename = 'ToLocalisation.csv'

#for i in range(16):
# Read data from CSV file
data = read_csv(filename)

# Process data and get position estimates
process_data(data)

#print(position_global)
#print(orientation)
#print(covariance)

# Define column names
columns = ['Px', 'Py', 'Pz']

# Convert nested list to DataFrame with column names
df = pd.DataFrame(position_global, columns=columns)

# Specify the file path
csv_file = 'imu_data_localized.csv'

# Write DataFrame to CSV file
df.to_csv(csv_file, index=False)


#time.sleep(dt)  # Adjust the delay as needed