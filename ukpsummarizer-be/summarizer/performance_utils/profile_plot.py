import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import numpy as np


path = "/home/orkan/Dropbox/measurements/sherlock/basis/"
files = ["01.csv", "02.csv"]
df1 = pd.read_csv(path + files[0], sep=",")
df2 = pd.read_csv(path + files[1], sep=",")

df1['class'] = pd.Series("Ausgangssystem", index=df1.index)
df2['class'] = pd.Series("mit Korrektur", index=df2.index)
df = pd.concat([df1, df2], ignore_index=True)

# sns.pointplot(x="sentences", y="t", color="salmon", ci=95, capsize=.2, data=df1)
# gr = sns.pointplot(x="sentences", y="t", color="forestgreen", ci=95, data=df2)
gr = sns.pointplot(x="sentences", y="t", hue="class", ci=95, data=df)

gr.set(yticks=np.arange(0, 111, 10))
plt.legend(title='lpSum Bugfix')
plt.grid(True, which="both")
plt.xlabel('Anzahl Sätze')
plt.ylabel('t in s')
plt.show()
