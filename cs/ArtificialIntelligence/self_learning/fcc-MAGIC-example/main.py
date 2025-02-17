

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt

cols = ["fLength","fWidth","fSize","fConc","fConc1",
            "fAsym","fM3Long","fM3Trans","fAlpha","fDist","class"]
df = pd.read_csv("magic04.data", names=cols)

# print 5 rows
print(df.head())

# 'g' and 'h'
print(df["class"].unique())

# convert 'g' and 'h' into 1 and 0
df["class"] = (df["class"] == "g").astype(int)

print((df["class"]).unique())

