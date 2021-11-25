
exports.handler = () => {return {statusCode: 418, body: "I'm a teapot!", headers: {'Content-Type': 'text/plain', 'Access-Control-Allow-Origin': '*'}}};