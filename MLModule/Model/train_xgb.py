#!/usr/bin/env python

import numpy as np
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score
import xgboost as xgb

# -------------------------
# Load dataset
# -------------------------
print("[*] Loading BODMAS dataset...")
data = np.load("../Dataset/bodmas.npz") 
X = data["X"]
y = data["y"]

print(f"Dataset loaded: {X.shape[0]} samples, {X.shape[1]} features")

# -------------------------
# Train/Test split
# -------------------------
X_train, X_test, y_train, y_test = train_test_split(
    X, y, test_size=0.2, random_state=42, stratify=y
)

# -------------------------
# Train XGBoost model
# -------------------------
print("[*] Training XGBoost...")
dtrain = xgb.DMatrix(X_train, label=y_train)
dtest = xgb.DMatrix(X_test, label=y_test)

params = {
    "objective": "binary:logistic",
    "eval_metric": "error",   # classification error
    "max_depth": 10,
    "eta": 0.1,
    "subsample": 0.8,
    "nthread": -1,
}

bst = xgb.train(params, dtrain, num_boost_round=200)

# -------------------------
# Evaluate model
# -------------------------
preds = (bst.predict(dtest) > 0.5).astype(int)
acc = accuracy_score(y_test, preds)
print(f"[*] XGBoost Test Accuracy: {acc:.4f}")

# -------------------------
# Save model
# -------------------------
bst.save_model("xgboost_bodmas.json")
print("[*] Model saved to xgboost_bodmas.json")
