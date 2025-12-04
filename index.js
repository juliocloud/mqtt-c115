const mqtt = require('mqtt');

const esp32Topic = 'esp32/temperature';

const client = mqtt.connect('mqtt://localhost');

client.on('connect', function () {
  console.log('Conectado com sucesso ao broker MQTT local');

  client.subscribe(esp32Topic, function (err) {
    if (!err) {
      console.log(`Inscrito no tópico '${esp32Topic}' e aguardando dados`);
    } else {
      console.error('Falha ao assinar o tópico:', err);
    }
  });
});

client.on('message', function (topic, message) {
  const messageString = message.toString();

  if (topic === esp32Topic) {
    console.log('---------------------------------');
    console.log(`Temperatura recebida do ESP32: ${messageString} °C`);
    console.log('---------------------------------');
  }
});

client.on('error', function (error) {
  console.error('Erro de conexão MQTT:', error);
});

setInterval(function () {
  const message = 'Script Node.js ativo às ' + new Date().toLocaleTimeString();
  client.publish('node/status', message);
}, 15000);
