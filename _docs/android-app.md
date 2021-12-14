---
title: "Android App"
permalink: /docs/android-app/
---


<div style="display: flex; flex-flow: row wrap; justify-content:center">
	<div style="flex-grow:1; flex-basis:65%;">
		<p>The last part of our system is a client’s app for smartphones, using which the vehicle owners can access the transferred information. From their point of view, it’s the most important part, as it’s the only thing they come to contact with. If the whole system should fulfill the role to which it was created, it is very important to have this app designed thoughtfully, with emphasis on a good <b>user experience (UX)</b>. Bad experience with the application would translate into dissatisfaction with the whole system and lack of interest in its use. Therefore, we placed a focus on simplicity, clear design, and the intuitiveness of UI.</p>
		<u>The functionalities we implemented are:</u>
		<ol>
			<li><b>Log in</b> with the owner’s account</li>
			<li>Showing the information from paired vehicles (<b>dashboard</b>)</li>
			<li>Viewing <b>historical statistics</b></li>
			<li>Viewing the vehicle <b>location and map</b> of the surrounding</li>
		</ol>
		<br>
		<p>We have created a native app for Android OS, written in <code>Java</code>, also with help of Google Firebase for some features. We tried to follow Google’s recommended design patterns, for example, the whole app is designed as <b>Single-Activity</b> (except login screen), with <code>Fragments</code> switching within it. Internally, we used the <b>MVVM architecture</b> with help of <code>ViewModels</code> and <code>LiveData</code>, as well as many other components from the <code>Android Jetpack</code> library. For easier communication between GUI components and the logical layer, the <code>View-binding</code> mechanism is used.</p>
	</div>
	<div style="flex-grow:0; flex-basis:35%; padding-top:0px; padding-left:20px; padding-right:20px; min-width:230px">
		<a href="{{ "/assets/img/docs/app1_full.jpg" | relative_url }}" data-lightbox="app-screens">
    		<img src="{{ "/assets/img/docs/app1.png" | relative_url }}" alt="app dashboard screenshot" style="max-width:100%; ">
		</a>
  </div>
</div>


### Screens
After the first launch, the <b><u>Login screen</u></b> is shown, enabling the user to log in (the registration process is not covered). For this, we use `Firebase Authentication`. The login is possible via email and password, or Google Account. After a successful logging in, the user information (list of paired vehicles, their attributes, and pictures) are downloaded from `Cloud Firestore` database and `Cloud Storage`, and the main <b><u>Dashboard screen</u></b> is shown. Tiles here are interactive, and after clicking them, they can perform some action (changing requested value and so on, this is just a preparation). On location tile, switching to <b><u>Map screen</u></b> is implemented. Interactive map view is made by using external `Google Maps SDK`, showing the last known vehicle location, user’s own location, and the map in a dark theme.  

By default, the last viewed vehicle is shown upon app launch, but the user can easily switch to other ones, using a rollable panel extendable from the bottom app bar. 
From here, it’s also possible to jump to the <b><u>Settings screen</u></b>, which is made using the `PreferenceScreen` component. Options to change user's nickname, log out, change the app language (<u>Slovak and English</u> **localizations** are available), modify vehicle, and a few other preferences are available. The selected options are stored in the `SharedPreferences` repository and in the local database, changes to vehicle attributes are instantly updated also online, into Firestore. The last fragment is the <b><u>History screen</u></b>, where the user can set date and time range using `SingleDateAndTimePicker` (external library), and after that, use an interactive graph to view data from the selected time range.

<div style="display:flex; flex-flow:row wrap; justify-content:center; gap: 20px">
	<div style="flex-basis:19rem">
		<a href="{{ "/assets/img/docs/app2_full.jpg" | relative_url }}" data-lightbox="app-screens">
			<img src="{{ "/assets/img/docs/app2.png" | relative_url }}" alt="app login screen">
		</a>
	</div>
	<div style="flex-basis:19rem">
		<a href="{{ "/assets/img/docs/app3_full.jpg" | relative_url }}" data-lightbox="app-screens">
			<img src="{{ "/assets/img/docs/app3.png" | relative_url }}" alt="app vehicle selector">
		</a>
	</div>
	<div style="flex-basis:19rem">
		<a href="{{ "/assets/img/docs/app4_full.jpg" | relative_url }}" data-lightbox="app-screens">
			<img src="{{ "/assets/img/docs/app4.png" | relative_url }}" alt="app history screen" style="max-width:100%;">
		</a>
	</div>
	<div style="flex-basis:19rem">
		<a href="{{ "/assets/img/docs/app5_full.jpg" | relative_url }}" data-lightbox="app-screens">
			<img src="{{ "/assets/img/docs/app5.png" | relative_url }}" alt="app settings screen">
		</a>
	</div>
	<div style="flex-basis:19rem">
		<a href="{{ "/assets/img/docs/app6_full.jpg" | relative_url }}" data-lightbox="app-screens">
			<img src="{{ "/assets/img/docs/app6.png" | relative_url }}" alt="app map screen">
		</a>
	</div>
	<div style="flex-basis:19rem">
		<a href="{{ "/assets/img/docs/app7_full.jpg" | relative_url }}" data-lightbox="app-screens">
			<img src="{{ "/assets/img/docs/app7.png" | relative_url }}" alt="app map screen">
		</a>
	</div>
</div>

### Under the hood
Aftet app startup, it pefrorms HTTP calls to the cloud’s REST API, asking for the last available statuses for all vehicles. Concurrently, the last status for the current vehicle available in a local cache is automatically shown, if it’s not older than a limit. Afterward, new statuses are received using FCM (push messaging), if allowed, or requested automatically in the set interval. All new received statuses are stored in the local cache (`SQLite` database, accessed using `Room` library). This is made in this way for two reasons - to have the last available status to quickly show it upon app launch, as well as to quickly show them in the graph in History screen. Additionally, an API is asked for all statuses from the requested time range, and they are added to the graph if any. During the request, a loading strip bar is showing the ongoing network operation. For working with the network and the backend API, we used `Retrofit` library, with `Gson` to (de)serialize JSON format.
If the charge level of any vehicle reaches the set target charge level, the app shows a system `notification`.

Some other features, for which the app is prepared, but we did not implement them for now, are:
- an ability to remotely control the vehicle (switching AC on/off, requested temperature, changing charging current, setting target charge level)
- an adjustment to more vehicle types (car, scooter, bike)
- the pairing process of a new vehicle
