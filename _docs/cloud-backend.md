---
title: "Cloud Backend"
permalink: /docs/cloud-backend/
---

Totally 3 clouds/services are participating in this project: 

### Sigfox Cloud
**Sigfox Cloud** is the main hub, or literally the backend of the Sigfox technology - all messages from all devices around the world end here. It is managed by Sigfox company itself and is placed in France territory. In the background, it solves message conflicts (broadcasts captured by more BTSs), checks the security, integrity, and authorization of messages. In the front, it sorts messages between user’s device groups and individual devices, calculates metrics, can decode messages payloads, and distributes them to next, external services. We use the *callback* mechanism, which invokes HTTP requests on external REST API, on each event, but also requesting data using its own API is possible. 

<u>In our case</u>, upon a new message from the vehicle is received, it is decompressed (reverse bite-shifts by custom grammar), JSON object is built, necessary HTTP headers are attached, and a request to our backend on AWS is made.

### AWS
**Amazon Web Services** is the main one.  
An `API Gateway` component is used to create the REST API. Its documentation can be found <a href="https://app.swaggerhub.com/apis-docs/martin195/EV-IoT-Kit/" target="_blank">on SwaggerHub UI</a>.  
Using it, new messages from endpoint devices can be uploaded, either directly from the device (if WLAN connectivity is used) or from Sigfox Cloud, and Android app instances can fetch the last available status or range of statuses for requested vehicle, and (un)register their tokens, using which they are addressed about assigned vehicles updates. In all cases, authorization using different <u>API keys</u> is required, and where's relevant, <u>access rights</u> (the ownership of requested vehicle) is evaluated using *Authorizer* mechanism on API calls.

The API methods invoke `Lambda functions`, written in <u>JavaScript</u>, which processes the given request. If a new vehicle update is received, it is stored to `DynamoDB`, what is a document-oriented <u>NoSQL</u> database. If the limit of saved statuses per that vehicle is reached, the oldest ones are deleted. Statuses are saved in originally received JSON format and are internally divided into database partitions according to the *vehicle’s ID*. Another Lambda function is triggered to find, if there’s a client app instance ID assigned to the vehicle, from which the update came from. These assignments are also saved in DynamoDB, in a simple key-value pair table. 

If match(es) are found, the user can have more devices, the status is finally ~~forwarded to `SNS` (Simple Notification Service) component~~ sent to *Google’s FCM endpoint*[^1].  
There are ofc also more lambda functions to process other requests from API, e.g. asking for the last status of a given device, a bunch of them for history view in app, etc.

### Google Firebase
**Firebase** is used indirectly, by the Android app. We use the `Authentication` component to provide login feature, `Firestore` database to save some information about vehicles (*user-vehicle registrations, user set vehicle name, license plate numbers, …*), and the `Cloud Storage`, the bucket-type storge, for storing the *vehicle’s title images*. The last piece is `Firebase Cloud Messaging` mechanism (FCM). It is an implementation of the <u>push-messaging</u> concept, using which the sender (cloud) can initiate the transfer and proactively deliver a new update to the addressed phone. Every app instance has its own unique *token*, which has to be delivered into the AWS backend, so the app can be targeted with the new updates of the associated vehicle(s), using it.

<br>

<u>A small note:</u>  
Google Firebase is actually a “subproduct” of *Google Cloud Platform* (GCP) - Google’s own public cloud, which is a direct competitor to AWS. Outwardly, Firebase looks to work independently, but under the hood it uses components of GCP. Therefore, the whole backend of our system could be potentially hosted in GCP instead of AWS, what could make communication with the client app more elegant, and pricing and managing of cloud services simpler. Vice-versa, AWS also offers a competitor service to Firebase - *AWS Amplify*.

<br>

[^1]: We have changed it to raw send to their new HTTP v1 API, cause we found AWS SNS is kinda cumbersome for this use case.
