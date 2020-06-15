from flask import render_template, Flask
from flask_sqlalchemy import SQLAlchemy
import os

project_dir = os.path.dirname(os.path.abspath(__file__))
database_file = "sqlite:///{}".format(os.path.join(project_dir, "Attendance.db"))

app = Flask(__name__)
app.config["SQLALCHEMY_DATABASE_URI"] = database_file
db = SQLAlchemy(app)

class User(db.Model):
    id = db.Column(db.Integer, unique=True, nullable=False)
    user_code = db.Column(db.Integer, unique=True, nullable=False)
    uid = db.Column(db.Text(30), unique=True, nullable=False)

db.create_all

@app.route('/')
def index():
    return "test"

@app.route('/insert/<user_id>/<uid>')
def ins(user_id, uid):
    new_user = User(user_code=user_id, uid=uid)
    db.session.add(new_user)
    db.sesison.commit()
    return render_template('insert.html', user_id=user_id, uid=uid)

@app.errorhandler(404)
def error(e):
    return "Error, page not found"



if __name__ == '__main__':
    app.run(debug=True, port=3333)