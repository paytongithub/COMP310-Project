import matplotlib.pyplot as plt

ys = []

f = open("malloc_log.txt", "r")

tmp = ""

for x in f:
  tmp = x
  if "Fragmentation" in  tmp:
      tmp = tmp.split()
      ys.append(float(tmp[-1]))

f.close()

plt.plot(range(len(ys)), ys, label = "Line")

plt.xlabel("x - Calls of simple_malloc or simple_free")
plt.ylabel("y - Fragmentation")
plt.title("Fragmentation of simple_malloc and simple_free in Bash")

plt.show()
