const AWS = require('aws-sdk');
const ddb = new AWS.DynamoDB.DocumentClient();

const dbTableName = "vehicle-statuses";
var lambda_reply = {headers: {'Content-Type': 'application/json', 'Access-Control-Allow-Origin': '*'}};

exports.handler = async function(event, context, callback)
{
    if(!event.hasOwnProperty('queryStringParameters'))
    {
        console.error("'queryStringParameters' attribute was not found while parsing the request");
        lambda_reply.statusCode = 500;
        callback(null, lambda_reply);
    }
    
    let qsp = event.queryStringParameters;
    if(!qsp.vehicleId || !qsp.from || !qsp.to) 
    {
        console.error("Missing basic attributes");
        lambda_reply.body = JSON.stringify({"Error": "Missing basic attributes"}, null, 2);
        lambda_reply.statusCode = 400;
        callback(null, lambda_reply);
    }
    
    //read from DynamoDB
    await readDocument(qsp.vehicleId, qsp.from, qsp.to).then(data =>
    {
        if(data.Items)
        {
            lambda_reply.body = JSON.stringify(data.Items, null, 2);
            lambda_reply.statusCode = 200;
            callback(null, lambda_reply);
        }
        else
        {
            lambda_reply.statusCode = 402;
            callback(null, lambda_reply);
        }
    }).catch((err) => console.error(err));
};

function readDocument(vehicleId, from, to)
{
    const request = 
    {
        TableName: 'vehicle-statuses',
        IndexName: 'vehicleId-timestamp-index',
        KeyConditionExpression: 'vehicleId = :vehicleId AND #timestamp BETWEEN :from AND :to',
        ExpressionAttributeNames: {"#timestamp": "timestamp"},
        ExpressionAttributeValues: {":vehicleId" : vehicleId, ":from" : parseInt(from), ":to" : parseInt(to)},
        ScanIndexForward: true //sort order
    }
    
    return ddb.query(request).promise();
}
