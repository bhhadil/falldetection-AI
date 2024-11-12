# /api/predict.py
import json
import tensorflow as tf
from flask import Flask, request
import numpy as np
import requests

# Charger le modèle LSTM
model = tf.keras.models.load_model('model/model.h5')

# Créer l'application Flask
app = Flask(__name__)

# Fonction pour envoyer une notification via Pushbullet
def send_push_notification(message):
    access_token = 'votre_clé_api_pushbullet'  # Remplacez par votre clé API Pushbullet
    url = 'https://api.pushbullet.com/v2/pushes'
    
    data = {
        'type': 'note',
        'title': 'Alerte Chute',
        'body': message
    }
    
    headers = {
        'Authorization': f'Bearer {access_token}',
        'Content-Type': 'application/json'
    }
    
    response = requests.post(url, json=data, headers=headers)
    if response.status_code == 200:
        print("Notification envoyée avec succès !")
    else:
        print("Erreur lors de l'envoi de la notification.")

# Définir la route Flask pour la prédiction
@app.route('/predict', methods=['POST'])
def predict():
    # Recevoir les données JSON envoyées par le client
    data = request.get_json()
    features = np.array(data['features']).reshape(1, -1)  # Adapter en fonction de vos données d'entrée
    
    # Prédire la chute (la sortie du modèle est un score)
    prediction = model.predict(features)
    
    # Si une chute est détectée, envoyer une notification Pushbullet
    if prediction > 0.5:  # Ajustez le seuil selon votre modèle
        send_push_notification("Chute détectée !")
    
    # Retourner la prédiction (0 ou 1) en format JSON
    result = {"prediction": int(prediction > 0.5)}
    return json.dumps(result)

# Pour démarrer le serveur Flask localement (en développement uniquement)
if __name__ == '__main__':
    app.run(debug=True)
