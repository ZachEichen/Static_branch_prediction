import lightgbm as lgb
import pandas as pd

model = lgb.Booster(model_file='lightgbm/lightgbm_branch_model.txt')

print(model.feature_importance)

test_data = pd.read_csv("test_data.csv")

predictions = model.predict(test_data)
print(predictions)