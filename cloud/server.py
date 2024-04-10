from flask import Flask
from flask import request
import psycopg2
import datetime

app = Flask(__name__)

conn = psycopg2.connect("dbname=smart_city host=localhost user=postgres password=password")

@app.route("/")
def hello_world():
    return "<p>Smart Parking Spaces Cloud Server</p>"

@app.route('/update', methods=['POST', 'GET'])
def set_parking_availability():
    error = None
    cur = conn.cursor()
    cur.execute("UPDATE parking_availability SET taken = %s, timestamp = %s WHERE space = %s", (request.args.get('taken', ''), datetime.datetime.now(), request.args.get('space', '')))
    conn.commit()
    return "<p>Updated</p>"

@app.route('/create', methods=['POST', 'GET'])
def create_parking_space():
    error = None
    cur = conn.cursor()
    cur.execute("INSERT INTO parking_availability (timestamp, space, taken) VALUES (%s, %s, %s)", (datetime.datetime.now(), request.args.get('space',''), request.args.get('taken','')))
    conn.commit()
    return "<p>Created</p>"

@app.route('/check', methods=['GET'])
def get_parking_availability():
    error = None
    cur = conn.cursor()
    cur.execute("SELECT * FROM parking_availability WHERE space = %s", request.args.get('space', ''))
    try:
        timestamp, space, taken = cur.fetchone()
        if taken == True:
            return "<p>Space {} is taken as of {}</p>".format(space, timestamp)
        else:
            return "<p>Space {} is free as of {}</p>".format(space, timestamp)
    except:
        return "<p>Space {} isn't in the database</p>".format(request.args.get('space', ''))
