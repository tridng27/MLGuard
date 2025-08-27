#!/usr/bin/env python

import os
import numpy as np
import ember
from sklearn.ensemble import RandomForestClassifier
from sklearn.metrics import accuracy_score
import xgboost as xgb
import tensorflow as tf
from tensorflow import keras

# ------------------------
# Load EMBER dataset
# ------------------------
def load_data(datadir, feature_version=2, n_samples=5000):
    """
    Load EMBER dataset (vectorized features).
    n_samples lets you limit dataset size for quick testing.
    """
    print("[*] Loading EMBER dataset...")
    X_train, y_train, X_test, y_test = ember.read_vectorized_features(datadir, feature_version)

    # Convert to numpy arrays
    X_train = X_train.toarray()
    X_test = X_test.toarray()

    # Downsample for testing (optional)
    if n_samples is not None:
        X_train = X_train[:n_samples]
        y_train = y_train[:n_samples]
        X_test = X_test[:n_samples]
        y_test = y_test[:n_samples]

    print(f"Loaded: {X_train.shape[0]} train samples, {X_test.shape[0]} test samples")
    return X_train, y_train, X_test, y_test


# ------------------------
# Train Random Forest
# ------------------------
def train_rf(X_train, y_train, X_test, y_test):
    print("\n[*] Training Random Forest...")
    rf = RandomForestClassifier(n_estimators=100, max_depth=20, random_state=42, n_jobs=-1)
    rf.fit(X_train, y_train)
    preds = rf.predict(X_test)
    acc = accuracy_score(y_test, preds)
    print(f"Random Forest Accuracy: {acc:.4f}")
    return rf


# ------------------------
# Train XGBoost
# ------------------------
def train_xgb(X_train, y_train, X_test, y_test):
    print("\n[*] Training XGBoost...")
    dtrain = xgb.DMatrix(X_train, label=y_train)
    dtest = xgb.DMatrix(X_test, label=y_test)

    params = {
        "objective": "binary:logistic",
        "eval_metric": "error",
        "max_depth": 10,
        "eta": 0.1,
        "subsample": 0.8,
    }

    bst = xgb.train(params, dtrain, num_boost_round=100)
    preds = (bst.predict(dtest) > 0.5).astype(int)
    acc = accuracy_score(y_test, preds)
    print(f"XGBoost Accuracy: {acc:.4f}")
    return bst


# ------------------------
# Train Deep Neural Network
# ------------------------
def train_dnn(X_train, y_train, X_test, y_test):
    print("\n[*] Training DNN...")
    model = keras.Sequential([
        keras.layers.Input(shape=(X_train.shape[1],)),
        keras.layers.Dense(512, activation="relu"),
        keras.layers.Dropout(0.3),
        keras.layers.Dense(256, activation="relu"),
        keras.layers.Dropout(0.3),
        keras.layers.Dense(1, activation="sigmoid"),
    ])

    model.compile(optimizer="adam", loss="binary_crossentropy", metrics=["accuracy"])
    model.fit(X_train, y_train, epochs=5, batch_size=64, validation_split=0.1, verbose=2)

    loss, acc = model.evaluate(X_test, y_test, verbose=0)
    print(f"DNN Accuracy: {acc:.4f}")
    return model


# ------------------------
# Main script
# ------------------------
if __name__ == "__main__":
    DATADIR = "../Dataset/ember"   # change this if needed

    X_train, y_train, X_test, y_test = load_data(DATADIR, n_samples=5000)

    rf_model = train_rf(X_train, y_train, X_test, y_test)
    xgb_model = train_xgb(X_train, y_train, X_test, y_test)
    dnn_model = train_dnn(X_train, y_train, X_test, y_test)

    print("\n[*] Training complete! Models are ready.")
