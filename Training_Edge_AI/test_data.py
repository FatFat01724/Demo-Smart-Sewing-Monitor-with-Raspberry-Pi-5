import joblib
import pandas as pd

# 1. Load the "AI Brain" (The pre-trained model)
try:
    # This file contains the Random Forest rules learned during training
    model = joblib.load('sewing_classifier.pkl')
    print("Model loaded successfully!")
except FileNotFoundError:
    print("Error: 'sewing_classifier.pkl' not found. Please run the training script first.")
    exit()

# 2. Simulate real-time data coming from the ESP32-S3
# Format: [[current, vibration, hall_sensor]]
test_data = [[3.5, 4.4, 10]]  # Example: High current, high vibration, high RPM

# 3. Convert input to a DataFrame (Matching the training format)
# The model expects feature names exactly as they were during fit()
input_df = pd.DataFrame(test_data, columns=['current', 'vibration', 'hall_sensor'])

# 4. Perform Inference (Prediction)
# This happens in milliseconds on the Raspberry Pi 5
prediction = model.predict(input_df)

# 5. Output the result
print(f"\n--- PREDICTION RESULT ---")
print(f"Input Data: {test_data}")
print(f"Predicted Sewing Stage: {prediction[0]}")
