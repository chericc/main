from flask import Flask, request, jsonify
import json

app = Flask(__name__,static_folder='static')

@app.route('/')
def index():
    return app.send_static_file('index.html')

@app.route('/submit', methods=['POST'])
def submit():
    # 从请求中获取 JSON 数据
    data = request.get_json()
    user_input = data.get('input')

    # 处理输入，构建响应
    response_message = f'You submitted: {user_input}'
    
    # 返回响应
    return jsonify({'message': response_message})

if __name__ == '__main__':
    app.run(host='0.0.0.0',port=8080,debug=True)
