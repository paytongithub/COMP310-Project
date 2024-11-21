import matplotlib.pyplot as plt

x1 = [1,2,3]
y1 = [2,4,1]

plt.plot(x1, y1, label = "Line")

plt.xlabel("x - Calls of simple_malloc or simple_free")
plt.ylabel("y - Fragmentation")
plt.title("Fragmentation of simple_malloc and simple_free in Bash")

plt.show()
