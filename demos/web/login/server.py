from flask import Flask, render_template, request, redirect, url_for

app = Flask(__name__)
app.secret_key = "your_secret_key_here"  # Needed for session management

# Hardcoded valid credentials (for demo purposes only!)
VALID_USERNAME = "admin"
VALID_PASSWORD = "secret"

@app.route('/')
def home():
    print('on home')
    return redirect(url_for('login'))

@app.route('/login', methods=['GET', 'POST'])
def login():
    print('on login')
    error = None
    if request.method == 'POST':
        username = request.form['username']
        password = request.form['password']
        
        if username == VALID_USERNAME and password == VALID_PASSWORD:
            return redirect(url_for('success'))
        else:
            error = 'Invalid credentials. Please try again.'
    
    return render_template('login.html', error=error)

@app.route('/success')
def success():
    print('on success')
    return render_template('success.html')

if __name__ == '__main__':
    app.run(host='0.0.0.0',port=5000,debug=True)