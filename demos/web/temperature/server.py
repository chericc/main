# app.py
from flask import Flask, render_template
import sqlite3
from datetime import datetime

app = Flask(__name__)

# 创建数据库和测试数据
def init_db():
    conn = sqlite3.connect('temperature.db')
    c = conn.cursor()
    c.execute('''CREATE TABLE IF NOT EXISTS temperatures
                 (date TEXT PRIMARY KEY, temperature REAL)''')
    
    # 插入示例数据（如果表为空）
    c.execute("SELECT COUNT(*) FROM temperatures")
    if c.fetchone()[0] == 0:
        sample_data = [
            ('2024-03-01', 15.2),
            ('2024-03-02', 16.8),
            ('2024-03-03', 18.5),
            ('2024-03-04', 17.1),
            ('2024-03-05', 19.3),
            ('2024-03-06', 20.0),
            ('2024-03-07', 19.7)
        ]
        c.executemany("INSERT INTO temperatures VALUES (?, ?)", sample_data)
        conn.commit()
    conn.close()

# 获取温度数据
def get_temperatures():
    conn = sqlite3.connect('temperature.db')
    c = conn.cursor()
    c.execute("SELECT date, temperature FROM temperatures ORDER BY date")
    data = c.fetchall()
    conn.close()
    
    # 转换为字典列表
    return [{'date': row[0], 'temperature': row[1]} for row in data]

@app.route('/')
def index():
    temperatures = get_temperatures()
    return render_template('index.html', temperatures=temperatures)

if __name__ == '__main__':
    init_db()
    app.run(debug=True, host='0.0.0.0', port=5000)