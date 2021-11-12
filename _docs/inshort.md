---
title: "In short"
permalink: /docs/inshort/
#redirect_from: /docs/
---

Originally, that's a practical part of my diploma thesis. The topic was proposed by *GlobalLogic Slovakia*, and I really liked the idea of a more complex system, from the field of IoT and electromobility, which has the potential to make the world smarter and greener.

### 💡 The idea behind this project.
Did you know that only 31% of potential customers want to buy an electric vehicle (EV) as their next car? While the prices and range of EVs are already on the acceptable level, the most feared factor is the <u>charging speed</u>, especially on trips. Although the parameters of batteries and their charging passibilities are improving significantly in the last years, this project is about the second thing we can do with this inconvenience - <u>make that process more comfortable.</u>

Whether it's at home or on trips, the vehicle’s owner needs to have the ability to check the charging progress and other things about his car, <u>remotely</u> - from his phone. And this needs to work all the time - regardless of the location, and without the need to connect anything manually. The customer wants to open an app and find, how far he can drive if he leaves right now, in 3 seconds. Every fail spoils the customer experience!

### ✎ What we designed.
<div style="display:flex; flex-flow:row wrap; justify-content:center">
  <div style="flex-grow:1; flex-basis:55%;">
    <p>The system, which sends the vehicle state to the cloud, where the data are stored and evaluated. Then, cloud proactively forwards this information to the owner’s smartphone through push messaging technology, to provide the most actual vehicle status to the user. We use redundant connectivity of end-devices, host the backend on a reliable public cloud, and have a native android app serving as the user’s client.</p>
    <p>After small changes in the android app, the project can be used also as a universal platform, not only for EVs, but also for any other (e.g. shared) vehicles, as electric scooters, motorcycles, e-bikes, or also non-smart devices as classic-bikes, with an additionally installable TCU.</p>
  </div>
  <div style="flex-grow:0; flex-basis:45%; min-width:300px">
    <a href="{{ "/assets/img/docs/diagram.png" | relative_url }}" data-lightbox="img">
      <img src="{{ "/assets/img/docs/diagram.png" | relative_url }}" alt="project architecture" style="max-width:100%; margin-top:15px; margin-left:20px;">
    </a>
  </div>
</div>

### ⚙️ How it works.
The project consists of 3 separate parts:

<ul class="nav nav-tabs" style="margin-top:20px;">
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
          <p>For more, check <a href="{{ "/docs/vehicle-simulator/" | relative_url }}">Vehicle Simulator article</a>, or <a href="{{ "/docs/sigfox/" | relative_url }}">this one about Sigfox</a>.</p>
      </div>
    </div>
  </div>

  <div class="tab-pane fade" id="cloud">
    <div class="bs-component">
        <div class="well">
            <h4>Cloud backend</h4>
            <p>Is hosted in <code>AWS (Amazon Web Services)</code>. We use a few components - an <b>API Gateway</b> which provides an REST API, <b>DynamoDB</b> - NoSQL database, to store actual and historical statuses of all vehicles, and <b>Lambda</b> functions which provides the runtime for service logic.</p>
            <p>Besides that, we use <code>Sigfox Cloud</code> as the source of messages from the Sigfox network, and <code>Google Firebase</code> for <b>push-messaging</b> and as support for some other app features. Ofc we use JSON as a communication format between APIs.</p>
            <p>A separate article - <a href="{{ "/docs/cloud-backend/" | relative_url }}">Cloud Backend</a>.</p>
        </div>
    </div>  
  </div>

  <div class="tab-pane fade" id="app">
    <div class="bs-component">
        <div class="well">
          <h4>Android app</h4>
          <p>Is written natively, in <code>Java</code>. We used Google’s recommended approach - the app is written as a <b>single activity</b>, with the <b>fragments</b> switching within it. <b>MVVM</b> architecture is used.</p>
          <p>After initial login through email or Google account, user can see a dashboard with the most recent information about his vehicle, browse data history with an interactive graph, or see vehicle’s location on built-in <code>Google Map</code> fragment. <b>Multiple vehicles</b> associated with one account are also supported. Internally, <code>SQLite</code> is used to store cached vehicles statuses; API requests to the cloud backend are made using <code>Retrofit</code> HTTP client. For user login functionality and for storing vehicles information, <code>Firebase Authentication</code> and <code>Firestore</code> are used.</p>
          <p>All other details are in <a href="{{ "/docs/android-app/" | relative_url }}">Android App article</a>.</p>
        </div>
    </div>
  </div>
</div>


What about reading the data from the vehicle itself? That was not part of my thesis. We expect a connection to vehicle's CAN bus or <abbr title="On-Board Diagnostics">OBD</abbr> port, but an official integration may be needed to obtain some information. Alternatively, some independent measuring of necessary values may be used. 

At last - this page, is written in `HTML/Markdown`, generated by `Jekyll` to a static website, and hosted on `GitHub Pages`.

<br>

### ▶️ Presentation

<u>What more</u> is in the theory of my thesis? An introduction to what **IoT**, **SmartCity**, **Industry 4.0**, and **Cloud technologies** means; comparison of more telecommunication technologies useful for IoT; a detailed look at **Sigfox** and **LoRa**; an explanation of cloud computing models and **virtualization** options; the principle of **push messaging**, and a more detailed explanation of how particular parts of the project are designed. 

* <a href="{{ "/assets/files/presentation.pdf" | relative_url }}" target="_blank">Download the presentation (.pdf)</a>  
* <a href="{{ "/assets/files/svos.pdf" | relative_url }}" target="_blank">Download the ŠVOS project (.pdf)</a>  
* <a href="{{ "/assets/files/DP-Timko-Martin-final.pdf" | relative_url }}" target="_blank">Download diploma thesis theory (.pdf)</a>, or from <a href="https://opac.crzp.sk/?fn=detailBiblioForm&sid=C5B0443D08DFEDB23D0F0949A386" target="_blank">official register of final theses (crzp.sk)</a>  
(Only in <attr title="Slovak">🇸🇰</attr>Slovak language, 106 pages.)

<div style="position:relative; padding-bottom:56.25%;">
  <iframe width="100%" height="100%" frameborder="0" style="position:absolute;"
  src="https://onedrive.live.com/embed?cid=B3E03C016AA72BE5&amp;resid=B3E03C016AA72BE5%21178&amp;authkey=AJv6Ej2hi_ofUSg&amp;em=2&amp;wdAr=1.777">
    Embedded presentation from <a target="_blank" href="https://office.com/">Microsoft Office 365</a>.
  </iframe>
</div>

<br>


