from flask import Flask, render_template

app = Flask(__name__)

@app.route('/')
def home():
    return render_template('index.html', message="Hello from Python and C!")

@app.route('/test')
def test():
    return render_template('test.html', message="Hello world!")

@app.route('/request', methods=['POST'])
def at_request():
    print('request')
    return 'request'

if __name__ == "__main__":
    app.run(debug=True)