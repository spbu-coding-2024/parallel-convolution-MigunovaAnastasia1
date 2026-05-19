import pandas as pd
import matplotlib.pyplot as plt

# Чтение данных из файла (укажите правильный путь к вашему CSV)
# Предполагается, что файл имеет заголовок: Queue size,Speedup
df = pd.read_csv("./../build/speedup.csv")   # или "ваш_файл.csv"

# Убедимся, что данные отсортированы по размеру очереди (по возрастанию)
df = df.sort_values("Queue size")

plt.figure(figsize=(10, 6))
plt.plot(df["Queue size"], df["Speedup"], marker='o', linestyle='-', linewidth=1.5, markersize=4)

plt.xlabel("Number of images processed (queue size)", fontsize=12)
plt.ylabel("Speedup (sequential / pipeline)", fontsize=12)
plt.title("Pipeline speedup vs number of images", fontsize=14)
plt.grid(True, linestyle='--', alpha=0.7)
plt.xticks(range(0, 23, 2))
plt.xlim(0, 23)
plt.ylim(bottom=0)
plt.tight_layout()

# Сохраняем график в файл
plt.savefig("speedup_plot.png", dpi=150)

# Показываем график (если нужно)
plt.show()