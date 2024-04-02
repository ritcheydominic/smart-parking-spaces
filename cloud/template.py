from flask import Flask
from flask import request
import psycopg2
import datetime

app = Flask(__name__)

conn = psycopg2.connect("dbname=template1 host=localhost user=postgres password=password")

@app.route("/")
def hello_world():
    print("Hello!")
    return "<p>Hello, world!</p>"

@app.route('/submit', methods=['POST', 'GET'])
def submit_data():
    error = None
    cur = conn.cursor()
    if request.method == 'POST':
        print("Is POST")
    cur.execute("INSERT INTO weather (timestamp, temperature, humidity) VALUES (%s, %s, %s)", (datetime.datetime.now(), request.args.get('temp',''), request.args.get('humi','')))
    conn.commit()
    return "<p>Submitted</p>"
