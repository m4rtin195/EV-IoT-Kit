const AWS = require('aws-sdk');
const fcm = require('./fcm-v1');
const decoder = require('./decoder');

const ddb = new AWS.DynamoDB.DocumentClient();

const dbTableNameStatuses = "vehicle-statuses";
const dbTableNameRegistrations = "fcm-registrations";
var lambda_reply = {headers: {'Content-Type': 'application/json', 'Access-Control-Allow-Origin': '*'}};

exports.handler = async function(event, context, main_callback)
{
    //console.log(event);
    
    /// REQUEST CHECK /// 
    if(!event.hasOwnProperty('body'))
    {
        lambda_reply.statusCode = 500;
        console.error("'body' attribute was not found while parsing the request");
        return lambda_reply;
    }
    
    let vStatus; //vehicleStatus
    try {
        vStatus = JSON.parse(event.body);
    }
    catch(e) {
        console.error(e);
        lambda_reply.statusCode = 400;
        lambda_reply.body = JSON.stringify({"Error": "Body parsing error"}, null, 2);
        main_callback(lambda_reply, null);
    }
    
    //let vStatus = event.body; //for AWS test
    
    if(!vStatus.vehicleId || !vStatus.timestamp)
    {
        console.error("Missing basic attributes");
        lambda_reply.body = JSON.stringify({"Error": "Missing basic attributes"}, null, 2);
        lambda_reply.statusCode = 400;
        main_callback(null, lambda_reply);
    }
    
    /// PROCESS STATUS TYPE///
    //set advanced to true etc. ?  // TODO
    if(vStatus.connectivity == 1) //sigfox
        vStatus.timestamp *= 1000;
    
    /// DECODING ///
    //convert halfs to floats
    if(vStatus.indoor_temperature) 
        vStatus.indoor_temperature = decoder.decodeHalf(vStatus.indoor_temperature);
    if(vStatus.outdoor_temperature) 
        vStatus.outdoor_temperature = decoder.decodeHalf(vStatus.outdoor_temperature);
    if(vStatus.desired_temperature) 
        vStatus.desired_temperature = decoder.decodeHalf(vStatus.desired_temperature);
    
    //decode location
    if(vStatus.location) 
        vStatus.location = decoder.decodeLocation(vStatus.location);
    
    /// WRITING TO DATABASE ///
    await createDocument(vStatus).then((reply) =>
    {
        if(Object.keys(reply).length === 0) //reply is empty
        {
            lambda_reply.statusCode = 201;
        }
        
        else
        {
            lambda_reply.statusCode = 500;
            lambda_reply.body = JSON.stringify({"Error": "Database write failed."}, null, 2);
            console.error(JSON.stringify(reply))
            main_callback(lambda_reply, null);
        }
    }).catch((err) => {console.error(err)});
    
    /// FCM ///
    //get tokens for the vehicle
    let tokens = await findFcmTokens(vStatus.vehicleId);
    if(tokens.length > 0)
    {
        //send message to all tokens
        await fcm.sendMessage(vStatus, tokens).then((reply) =>
        {
            if(reply !== 200)  //delivery failed
            {
                lambda_reply.body = JSON.stringify({
                    "Database": lambda_reply.statusCode.toString(),
                    "FCM": reply
                }, null, 2);
                lambda_reply.statusCode = 207; //Multi-Status
            }
        }, (error) => //http failed
        {
            //console.error is already in sendMessage()
            lambda_reply.statusCode = 500;
            lambda_reply.body = JSON.stringify({"Error": "FCM calls failed."}, null, 2);
            main_callback(lambda_reply, null);
        });
    }
    
    // END, return reply ///
    //console.log(lambda_reply)
    main_callback(null, lambda_reply);
};


function createDocument(document)
{
    document.id = generateDocumentID();
    const request = 
    {
        TableName: dbTableNameStatuses,
        Item: document
    }
    return ddb.put(request).promise();
}

function generateDocumentID() 
{
    // Math.random should be unique because of its seeding algorithm.
    // Convert it to base 36 (numbers + letters), and grab the first 9 characters after the decimal.
    return Math.random().toString(36).substr(2,9);
};

var findFcmTokens = async function(vehicleId)
{
    let tokens = [];
    let query = 
    {
        TableName: dbTableNameRegistrations,
        //AttributesToGet: ['fcmToken'], //nemoze byt v jednom query spolu s KeyConditionExpression facepalm
        KeyConditionExpression: 'vehicleId = :vehicleId',
        ExpressionAttributeValues: {":vehicleId" : vehicleId}
    }

    await ddb.query(query).promise().then(data => 
    {
        if(data.Count > 0) 
            data.Items.forEach(a => tokens.push(a.fcmToken))
    });

    return tokens;
}
