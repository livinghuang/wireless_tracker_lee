from flask import Flask, jsonify, request

app = Flask(__name__)

@app.route('/', methods=['POST'])
def receive_data():
    # Check if the incoming request is JSON
    if request.is_json:
        # Parse the JSON data
        data = request.get_json()
        
        # You can now use the `data` dictionary as needed
        print(data)  # Example: Print data to the console
        
        # Respond to the client that the data was received successfully
        return jsonify({"message": "Data received successfully", "yourData": data}), 200
    else:
        return jsonify({"error": "Request must be JSON"}), 400

if __name__ == '__main__':
    app.run(debug=True, port=5000)
