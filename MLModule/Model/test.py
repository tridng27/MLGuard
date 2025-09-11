import numpy as np
from xgboost import XGBClassifier
from sklearn.metrics import classification_report

# Load dataset
data = np.load("../Dataset/bodmas.npz")
X, y = data["X"], data["y"]

# Load trained model
model = XGBClassifier()
model.load_model("xgboost_bodmas.json")

# Pick 10 random samples
idx = np.random.choice(len(X), 10, replace=False)
preds = model.predict(X[idx])

print("Indices:", idx)
print("Predictions:", preds)
print("Ground truth:", y[idx])

y_pred = model.predict(X)
print(classification_report(y, y_pred, target_names=["Benign", "Malicious"]))

