---
title: "Android App"
permalink: /docs/android-app/
---


<div style="display: flex; flex-flow: row wrap;">
	<div style="flex-basis:65%; min-width:400px">
		<p>From a owner’s point of view, a client’s app is the most important part of the system. It’s his interface, the only thing he comes to contact with. 
		We placed a focus on simplicity and clear design. There are XXXXX
		The last part of our system is a client’s app for smartphones, using which the vehicle owners can access the transferred information. From their point of view, it’s the most important part, as it’s the only part they come to contact with. If the whole system should fulfill the role to which it was created, it is very important to have this app designed thoughtfully, with emphasis on a good <b>user experience (UX)</b>. Bad experience with the application would translate into dissatisfaction with the whole system and lack of interest in its use. The application environment must therefore be clear and intuitive.</p>
		<u>The functionalities we implemented are:</u>
		<ol>
			<li><b>Log in</b> with the owner’s account</li>
			<li>Showing the information from paired vehicles (<b>dashboard</b>)</li>
			<li>Viewing <b>historical statistics</b></li>
			<li>Viewing the vehicle <b>location and map</b> of the surrounding</li>
		</ol>
		<br>
		<p>We have created a native app for Android OS, written in <code>Java</code>, also with help of Google Firebase for some features. We tried to follow Google’s recommended design patterns, for example, the whole app is designed as <b>Single-Activity</b> (except login screen), with <code>Fragments</code> switching within it. Internally, we used the <b>MVVM architecture</b> with help of <code>ViewModels</code>, as so many others components from the <code>Android Jetpack</code> library. For easier communication between GUI components and logical layer, <code>View-binding</code> mechanism is used.</p>
	</div>
	<div style="flex-basis:35%;">
		<a href="{{ "/assets/img/docs/app1_full.jpg" | relative_url }}" data-lightbox="app-screens">
    		<img src="{{ "/assets/img/docs/app1.png" | relative_url }}" alt="app dashboard screenshot" style="padding-top:0px; padding-left:20px; padding-right: 20px; max-width:100%;">
		</a>
  </div>
</div>


### Screens
After first launch, the <b><u>Login screen</u></b> is showed, enabling the user to log-in (registration process is not covered). For this, we use `Firebase Authentication`. The login is possible via email and password, or Google Account. After successful logging in, the user information (list of paired vehicles, their attributes and picture) are downloaded from `Firebase Firestore` database and `Cloud Storage`, and main <b><u>Dashboard screen</u></b> is showed. Tiles here are interactive, and after clicking them, they can perform some action (changing requested value and so on, this is just preparation). On location tile, switching to <b><u>Map screen</u></b> is implemented.  
Interactive map view is made by using external `Google Maps SDK`, showing last known vehicle location, user’s own location, and the map in a dark theme.
By default, the last viewed vehicle is shown upon app launch, but the user can easily switch to other ones, using a rollable panel extendable from the bottom appbar. 
From here, it’s also possible to jump to <b><u>Settings screen</u></b>, which is made using `PreferenceScreen` mechanism. The options to change user nickname, logging out, changing the app language (Slovak and English localizations are available), and few other preferencies are available. The selected options are stored in the `SharedPreferences` repository and in the local database. 
The last fragment is <b><u>History screen</u></b>, where user can set date and time range using `SingleDateAndTimePicker` (external library), and after that, use interactive graph to view data from selected time range.

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
</div>

### Under the hood
Aftet app startup, it pefrorm HTTP calls to cloud’s REST API, asking for last available statuses for all vehicles. Concurrently, the last status for the current vehicle available in a local cache is automatically shown, if it’s not older than a limit. Afterwards, new statuses are received using FCM (push messaging), if allowed, or requested automatically in the set interval. All new received statuses are stored in the local cache (`SQLite` database, acessed using `Room` library). This is made in this way for two reasons - to have the last available status to quickly show it upon app launch, as well to quickly show the them in graph in History screen. Additionaly, an API is asked for all statuses from requested time range, and they are added to the graph if any. During the request, loading strip bar is showing the ongoing network operation. For working with network and backend API, we used `Retrofit` library with `Gson` to (de)serialize JSON format.
In case that charge level of some vehicle reach the set target charge level, the app shows a system `notification`.

Some other features, for which the app is prepared, but we did not implemented them for now, are: 
- an ability to remotely control the vehicle (switching AC on/off, requested temperature, changing charging current, setting targert charge level)
- an adjustment to more vehicle types (car, scooter, bike)
- the pairing process of a new vehicle


///
Our requirements from the beginning of designing it were:
- simplicity
- availability
- reliability
The difference between last two?