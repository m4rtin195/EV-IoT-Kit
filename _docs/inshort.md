---
title: "In short"
permalink: /docs/inshort/
#redirect_from: /docs
---

EV-IoT-Kit is a 
Actually, this is a practical part of my diploma thesis. The topic of it was proposed by GlobalLogic Slovakia, and I really liked the idea of a more complex system, from the field of IoT and electromobility, which has the potential to make the world smarter and greener.

## The idea behind this project.
Did you know that only 31% of potential customers would buy an electric vehicle (EV) as their next car? While the price and approach of EVs are already on the acceptable level, the most feared factor is the charging speed, on trips. Although the parameters of batteries and their charging passibilities are improving significantly in the last years, this project is about the second thing we can do with this inconvenience - make that process more comfortable. 
Whether it's at home or on the trips, the vehicle’s owner needs to have an ability to check charging progress and other things about his car, remotely - from his phone. And this need to work all the time - regardless the location and without need to connect anything manually. The customer wants to open an app and find how far he can drive if he leaves right now, in the 3 seconds. Every fail spoils the customer experience!

## What we designed.
The system, which sends the vehicle state to the cloud, where the data are stored and evaluated. Then, cloud proactively forward this information to the owner’s smartphone through push messaging technology, to provide the most actual vehicle status to the user. We use redundant connectivity of end-devices, host the backend on a reliable public cloud and have an native android app serving as the user’s client.

After small changes in the android app, the project can be used also as a universal platform, not only for EVs, but also for any other (e.g. shared) vehicles, as electric scooters, motocycles, e-bikes, or also non-smart devices as classic-bikes, with a additionally installable TCU. 

## How it works.
The project consists of 3 independent parts:
- Vehicle simulator
- Cloud backend
- Android app

### 1. Vehicle simulator
I don't own an electric car yet. Had to make smaller compromises.
It is simply a Raspberry Pi running a software which simulates various parameters about vehicle’s state (charge level, remaining time, approach, …), and which sends these values to the cloud backend.
An app is written in C++ and Qt, tries to copy a real vehicle behavior and has the simple GUI to control it. To transfer information to cloud, we use two ways - ordinary internet connection (through Wi-Fi) if it is available, or Sigfox - IoT communication technology which provides great availability in many countries.
<div class=“row”>
  <div style="float:left; width:50%">toto je text</div>
  <div style="float:left; width:50%">toto je image1</div>
</div>
For more, check ~(link na clanok)~, or ~(this one about Sigfox)~.

### 2. Cloud backend
Is hosted in AWS (Amazon Web Services). We use a few components - an *API Gateway* which provides an REST API, *DynamoDB* - NoSQL database, to store actual and historical statuses of all vehicles, and *Lambda* functions which provides the runtime for service logic. 
Besides that, we use Sigfox Cloud as the source of messages from the Sigfox network, and Google Firebase for push-messaging and as support for some other app features. Ofc we use JSON as a communication format between APIs.
A separate article - ~(link na clanok)~.

### 3. Android app
Is written natively, in Java. We used Google’s recommended approach - the app is written as a single activity, with the fragments switching within it. After initial login through email or Google account, user can see the main dashboard with most recent information about his vehicle, browse data history with interactive graph, or see vehicle’s location on built-in Google Map fragment. Multiple vehicles associated with one account are also supported. Internally, *SQLite* is used to store cached vehicles statuses, API requests to the cloud backend are made using *Retrofit* HTTP client. For user login and storing vehicles information, Firebase Authentication and Firestore are used. 
All other are details are in ~(link na clanok)~.

![](In%20short/53CF2A89-32B0-4ECF-BF3E-13C49CFA2C02.png)

used technologies image

What about reading the data from the vehicle’s electronic system itself? That was not part of my diploma thesis. We expect an official integration into the vehicle’s systems, or using some independent measuring of requested values. 

What more is in the theory of my thesis? Introduction to what IoT, SmartCity, Industry 4.0, and Cloud technologies means; Comparation of many wireless telecommunication technologies useful for IoT, detailed look at the Sigfox, 

Download the presentation (.pdf)  
Download the soc projekt (.pdf)  
Download diploma thesis theory (.pdf), or from official register of final theses.  Only in Slovak language, 106 pages.

At last - this page, is wrote in `Markdown`, generated by `Jekyll` to the static website and hosted in `GitHub Pages`.

## Presentation

<iframe src="https://onedrive.live.com/embed?cid=B3E03C016AA72BE5&amp;resid=B3E03C016AA72BE5%21178&amp;authkey=AJv6Ej2hi_ofUSg&amp;em=2&amp;wdAr=1.7777777777777777" width="860px" height="480px" frameborder="0">Embedded presentation from <a target="_blank" href="https://office.com/">Microsoft Office 365</a>.</iframe>