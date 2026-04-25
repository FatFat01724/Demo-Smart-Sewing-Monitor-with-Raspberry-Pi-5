import pandas as pd
from sklearn.model_selection import train_test_split
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import classification_report
import joblib

# 1. Load data
try:
    df = pd.read_csv('sewing_data.csv')
    print("Data loaded successfully!")
except FileNotFoundError:
    print("Error: sewing_data.csv not found!")
    exit()

# 2. Features and Labels
X = df[['current', 'vibration', 'hall_sensor']]
y = df['stage']

# 3. Split data
X_train, X_test, y_train, y_test = train_test_split(X, y, test_size=0.2, random_state=42)

# 4. Train Random Forest 
model = RandomForestClassifier(n_estimators=100, n_jobs=-1, random_state=42)
model.fit(X_train, y_train)

# 5. Evaluate
y_pred = model.predict(X_test)
print("\n--- Evaluation Report ---")
print(classification_report(y_test, y_pred))

# 6. Save model for Edge AI Inference
joblib.dump(model, 'sewing_classifier.pkl')
print("\nModel saved as: sewing_classifier.pkl")
