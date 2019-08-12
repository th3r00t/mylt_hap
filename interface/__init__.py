from flask import Flask
app = Flask(__name__)
app.config.from_object('config')
app.config["DEBUG"] = True
@app.route("/")
def hello():
    return "Hello, Flask"
if __name__ == "__main__":
    app.run()
