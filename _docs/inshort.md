---
title: "In short"
permalink: /docs/inshort/
#redirect_from: /docs
---

EV-IoT-Kit is a 
Actually, this is a practical part of my diploma thesis. The topic of it was proposed by GlobalLogic Slovakia, and I really liked the idea of a more complex system, from the field of IoT and electromobility, which has the potential to make the world smarter and greener.

### 💡 The idea behind this project.
Did you know that only 31% of potential customers would buy an electric vehicle (EV) as their next car? While the price and approach of EVs are already on the acceptable level, the most feared factor is the charging speed, on trips. Although the parameters of batteries and their charging passibilities are improving significantly in the last years, this project is about the second thing we can do with this inconvenience - make that process more comfortable. 
Whether it's at home or on the trips, the vehicle’s owner needs to have an ability to check charging progress and other things about his car, remotely - from his phone. And this need to work all the time - regardless the location and without need to connect anything manually. The customer wants to open an app and find how far he can drive if he leaves right now, in the 3 seconds. Every fail spoils the customer experience!

### ✎ What we designed.
The system, which sends the vehicle state to the cloud, where the data are stored and evaluated. Then, cloud proactively forward this information to the owner’s smartphone through push messaging technology, to provide the most actual vehicle status to the user. We use redundant connectivity of end-devices, host the backend on a reliable public cloud and have an native android app serving as the user’s client.

After small changes in the android app, the project can be used also as a universal platform, not only for EVs, but also for any other (e.g. shared) vehicles, as electric scooters, motocycles, e-bikes, or also non-smart devices as classic-bikes, with a additionally installable TCU. 

### ⚙️ How it works.
The project consists of 3 independent parts:

<br>
<ul class="nav nav-tabs">
  <li class="active"><a href="#simulator" data-toggle="tab">1. Vehicle simulator</a></li>
  <li><a href="#cloud" data-toggle="tab">2. Cloud backend</a></li>
  <li><a href="#app" data-toggle="tab">3. Android app</a></li>
</ul>

<div id="myTabContent" class="tab-content">
  <div class="tab-pane fade active in" id="simulator">
    <div class="bs-component">
        <div class="well">
          <h4>Vehicle simulator</h4>
          <p>I don't own an electric car yet. Had to make smaller compromises.
          It is simply a <code>Raspberry Pi</code> in a nice box with a touch display, running a <b>custom software</b> that simulates various parameters about the vehicle’s state (<i>charge level</i>, <i>remaining time</i>, <i>range</i>, …), and which sends these values to the cloud backend.</p>
          <p>An app is written in <code>C++</code> and <code>Qt</code>, tries to copy a real vehicle behavior and has the <b>simple GUI</b> to control it. To transfer information to cloud, we use two ways - ordinary <b>network</b> connection (through Wi-Fi) <u>if it is available</u>, or <code>Sigfox</code> - <abbr title="Internet of Things">IoT</abbr> communication technology which provides great availability in many countries.</p>
          <div>
            <div style="float:left; width:50%">toto je text</div>
            <div style="float:left; width:50%">toto je image1</div>
          </div>
          <p>For more, check <a href="/docs/vehicle-simulator/">Vehicle Simulator article</a>, or <a href="/docs/sigfox/">this one about Sigfox</a>.</p>
      </div>
    </div>
  </div>

  <div class="tab-pane fade" id="cloud">
    <div class="bs-component">
        <div class="well">
            <h4>Cloud backend</h4>
            <p>Is hosted in <code>AWS (Amazon Web Services)</code>. We use a few components - an <b>API Gateway</b> which provides an REST API, <b>DynamoDB</b> - NoSQL database, to store actual and historical statuses of all vehicles, and <b>Lambda</b> functions which provides the runtime for service logic.</p>
            <p>Besides that, we use <code>Sigfox Cloud</code> as the source of messages from the Sigfox network, and <code>Google Firebase</code> for <b>push-messaging</b> and as support for some other app features. Ofc we use JSON as a communication format between APIs.</p>
            <p>A separate article - <a href="/docs/cloud-backend/">Cloud Backend</a>.</p>
        </div>
    </div>  
  </div>

  <div class="tab-pane fade" id="app">
    <div class="bs-component">
        <div class="well">
          <h4>Android app</h4>
          <p>Is written natively, in <code>Java</code>. We used Google’s recommended approach - the app is written as a <b>single activity</b>, with the <b>fragments</b> switching within it.</p>
          <p>After initial login through email or Google account, user can see the main dashboard with most recent information about his vehicle, browse data history with interactive graph, or see vehicle’s location on built-in <code>Google Map</code> fragment. Multiple vehicles associated with one account are also supported. Internally, <code>SQLite</code> is used to store cached vehicles statuses, API requests to the cloud backend are made using <code>Retrofit</code> HTTP client. For user login and storing vehicles information, <code>Firebase Authentication</code> and <code>Firestore</code> are used.</p>
          <p>All other details are in <a href="/docs/android-app/">Android App article</a>.</p>
        </div>
    </div>
  </div>
</div>

<br>

![](In%20short/53CF2A89-32B0-4ECF-BF3E-13C49CFA2C02.png)

used technologies image

What about reading the data from the vehicle’s electronic system itself? That was not part of my thesis. We expect an official integration into the vehicle’s systems, or using some independent measuring of necessary values. 

<br>

What more is in the theory of my thesis? Introduction to what <b>IoT</b>, <b>SmartCity</b>, <b>Industry 4.0</b>, and <b>Cloud technologies</b> means; Comparation of many wireless telecommunication technologies useful for IoT, detailed look at <b>Sigfox</b>, DOKONCIT 

* ▶️ <a href="/assets/files/presentation.pdf" target="_blank">Download the presentation (.pdf)</a>  
* ▶️ <a href="/assets/files/svos.pdf" target="_blank">Download the ŠVOS project (.pdf)</a>  
* ▶️ <a href="/assets/files/DP-Timko-Martin-final.pdf" target="_blank">Download diploma thesis theory (.pdf)</a>, or from <a href="https://opac.crzp.sk/?fn=detailBiblioForm&sid=C5B0443D08DFEDB23D0F0949A386" target="_blank">official register of final theses (crzp.sk)</a>  
Only in <attr title="Slovak">🇸🇰</attr> Slovak language, 106 pages.

At last - this page, is wrote in `Markdown`, generated by `Jekyll` to the static website and hosted in `GitHub Pages`.

### Presentation

<iframe src="https://onedrive.live.com/embed?cid=B3E03C016AA72BE5&amp;resid=B3E03C016AA72BE5%21178&amp;authkey=AJv6Ej2hi_ofUSg&amp;em=2&amp;wdAr=1.7777777777777777" width="860px" height="480px" frameborder="0">Embedded presentation from <a target="_blank" href="https://office.com/">Microsoft Office 365</a>.</iframe>