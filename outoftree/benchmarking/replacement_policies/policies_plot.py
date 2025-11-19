import matplotlib.pyplot as plt
import pandas as pd
w_out_loc = pd.read_csv("withoutlocality.csv")
wld_80_20 = pd.read_csv("80_20pWorkload.csv")
loop_seq  = pd.read_csv("Looping_sequential.csv")

fig, axes = plt.subplots(nrows=1, ncols=3, figsize=(15, 6), sharex=True, sharey=True)
axes = axes.flatten()

ax = axes[0]

ax.plot(w_out_loc['X'],w_out_loc['OPT'],label="OPT",color='r')
ax.plot(w_out_loc['X'],w_out_loc['FIFO'],label="FIFO",color='b')
ax.plot(w_out_loc['X'],w_out_loc['LRU'],label="LRU",color='g')
ax.plot(w_out_loc['X'],w_out_loc['RANDOM'],label="RANDOM",color='orange')
ax.plot(w_out_loc['X'],w_out_loc['APPROXLRU'],label="ApproxLRU",color='black')

ax.set_xlabel("Cache Sizes")
ax.set_ylabel("Hit rates")

ax.set_title("The No-Locality Workload")

ax.legend()

ax1 = axes[1]
ax1.plot(wld_80_20['X'],wld_80_20['OPT'],label="OPT",color='r')
ax1.plot(wld_80_20['X'],wld_80_20['FIFO'],label="FIFO",color='b')
ax1.plot(wld_80_20['X'],wld_80_20['LRU'],label="LRU",color='g')
ax1.plot(wld_80_20['X'],wld_80_20['RANDOM'],label="RANDOM",color='orange')
ax1.plot(wld_80_20['X'],wld_80_20['APPROXLRU'],label="ApproxLRU",color='black')

ax1.set_xlabel("Cache Sizes")
ax1.set_ylabel("Hit rates")

ax1.set_title("The 80-20 Workload")

ax1.legend()

ax2 = axes[2]

ax2.plot(loop_seq['X'],loop_seq['OPT'],label="OPT",color='r')
ax2.plot(loop_seq['X'],loop_seq['FIFO'],label="FIFO",color='b')
ax2.plot(loop_seq['X'],loop_seq['LRU'],label="LRU",color='g')
ax2.plot(loop_seq['X'],loop_seq['RANDOM'],label="RANDOM",color='orange')
ax2.plot(loop_seq['X'],loop_seq['APPROXLRU'],label="ApproxLRU",color='black')

ax2.set_xlabel("Cache Sizes")
ax2.set_ylabel("Hit rates")

ax2.set_title("The Looping-Sequential Workload")

ax2.legend()
plt.tight_layout()
plt.show()