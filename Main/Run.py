import os
import sys
import glob
import json
import numpy as np
import xgboost as xgb

# ------------------ Paths ------------------ #
BASE_DIR = os.path.dirname(os.path.abspath(__file__))
EMBER_PATH = os.path.join(BASE_DIR, "../ember/ember")  # points to ember/features.py directory
sys.path.insert(0, EMBER_PATH)

MODEL_PATH = os.path.join(BASE_DIR, "../MLModule/Model/xgboost_bodmas.json")
DOWNLOADS_PATH = os.path.expanduser("~/Downloads")  # adjust if needed

# ------------------ Import EMBER Feature Extractor ------------------ #
from features import PEFeatureExtractor

# ------------------ Load XGBoost Model ------------------ #
xgb_model = xgb.XGBClassifier()
xgb_model.load_model(MODEL_PATH)

# ------------------ Helper Functions ------------------ #
def find_pe_files(folder):
    """Recursively find PE files in a folder."""
    extensions = ["*.exe", "*.dll", "*.sys", "*.cpl", "*.ocx"]
    files = []
    for ext in extensions:
        files.extend(glob.glob(os.path.join(folder, "**", ext), recursive=True))
    return files

def extract_features(file_path, extractor):
    """Extract features using EMBER PEFeatureExtractor."""
    with open(file_path, "rb") as f:
        data = f.read()
    feats = extractor.feature_vector(data)
    return feats.reshape(1, -1)

# ------------------ Main ------------------ #
def main():
    extractor = PEFeatureExtractor(feature_version=2)
    pe_files = find_pe_files(DOWNLOADS_PATH)

    if not pe_files:
        print("No PE files found in Downloads.")
        return

    for pe_file in pe_files:
        try:
            features = extract_features(pe_file, extractor)
            pred = xgb_model.predict(features)
            prob = xgb_model.predict_proba(features)[0][1]  # malware probability
            print(f"{pe_file}: Prediction={pred[0]}, Malware_Prob={prob:.4f}")
        except Exception as e:
            print(f"Error processing {pe_file}: {e}")

if __name__ == "__main__":
    main()
