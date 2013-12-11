var states = {
    START: 0,
    STOP: 1,
    PREVPAGE: 2,
    NEXTPAGE: 3,
    CHANGEDISPLAY: 4,
    REFRESH: 5
};

var displays = {
    PRICE: 0,
    CHANGE: 1,
    PERCENT: 2
};

var currentDisplay = displays.PRICE;
var interval = 1;

var host = 'query.yahooapis.com',
    path = '/v1/public/yql',
    queryString = '',
    queryURL = '';


//for timer 
var refreshIntervalId;    

var yql_query_template = "select * from yahoo.finance.quote where symbol in (\"$0\",\"$1\",\"$2\",\"$3\")";
var yql_query = "";
var params = {};

function constructYQL(stocks){
    var yql_query = yql_query_template;
	for (var i=0;i<stocks.length;i++){
		yql_query = yql_query.replace("$" + i , stocks[i] );
	}
	
	// Required YQL paramaters
	params.env    = params.env !== undefined ? params.env : 'http://datatables.org/alltables.env';
	params.format = params.format !== undefined ? params.format : 'json';
	params.q      = yql_query;
}

// This utility function creates the query string
// to be appended to the base URI of the YQL Web
// service.
function toQueryString(obj) {    
  var parts = [];    
  for(var each in obj) if (obj.hasOwnProperty(each)) {
    parts.push(encodeURIComponent(each) + '=' + encodeURIComponent(obj[each]));    
  }    
  return parts.join('&');  
}

function constructURL(){
	console.log("Construct Query URL");
	var stockObjects = JSON.parse(window.localStorage.getItem('stocks'));
	console.log(stockObjects);
	constructYQL(stockObjects);
	queryString = toQueryString(params);
	queryURL = 'http' + '://' + host + path + '?' + queryString;
	console.log(queryURL);
}

function start(){
	console.log("Start Fetching");
	fetchQuote();
	refreshIntervalId = setInterval(function(){fetchQuote()},interval*60000);
}

function stop(){
	console.log("Stop Fetching");
	clearInterval(refreshIntervalId);
}	

function fetchQuote() {
	console.log("Fetch Quote");
  var response;
  var req = new XMLHttpRequest();
  //req.open('GET', 'http://query.yahooapis.com/v1/public/yql?q=select%20*%20from%20yahoo.finance.quote%20where%20symbol%20in%20(%22YHOO%22%2C%22AAPL%22%2C%22GOOG%22%2C%22MSFT%22)&format=json&diagnostics=true&env=store%3A%2F%2Fdatatables.org%2Falltableswithkeys&callback=', true);
  req.open('GET', queryURL, true);
  req.onload = function(e) {
    if (req.readyState == 4) {
      if(req.status == 200) {
        console.log(req.responseText);
        response = JSON.parse(req.responseText);
  		var stocks = response.query.results.quote;
  		
  		var change = new Array();
  		var price = new Array();
  		var percent = new Array();
  		
  		for (var i=0;i<4;i++)
		{
			
			price[i] = parseFloat(stocks[i].LastTradePriceOnly).toFixed(2);
			if(stocks[i].Change >= 0){
				change[i] = "+" + parseFloat(stocks[i].Change).toFixed(2);
				percent[i] = "+" + ((parseFloat(stocks[i].Change)/price[i])*100).toFixed(2) + "%";
			}else{
				change[i] = parseFloat(stocks[i].Change).toFixed(2);
				percent[i] = ((parseFloat(stocks[i].Change)/price[i])*100).toFixed(2) + "%";
			}
			
		}
		
		if(currentDisplay == displays.PRICE){
			Pebble.sendAppMessage({ "0" : stocks[0].symbol, "1" : price[0], "2" : stocks[1].symbol, "3" : price[1], "4" :stocks[2].symbol, "5" : price[2], "6" :stocks[3].symbol, "7" : price[3]});
      	}else if(currentDisplay == displays.CHANGE){
      		Pebble.sendAppMessage({ "0" : stocks[0].symbol, "1" : change[0], "2" : stocks[1].symbol, "3" : change[1], "4" :stocks[2].symbol, "5" : change[2], "6" :stocks[3].symbol, "7" : change[3]});
      	}else{
      		Pebble.sendAppMessage({ "0" : stocks[0].symbol, "1" : percent[0], "2" : stocks[1].symbol, "3" : percent[1] , "4" :stocks[2].symbol, "5" : percent[2], "6" :stocks[3].symbol, "7" : percent[3]});
      	}		
      } else {
        console.log("Error");
      }
    }
  }
  req.send(null);
}

Pebble.addEventListener("ready",
                        function(e) {
                          console.log("connect!" + e.ready);
                          //check if stocks is set before
                          /*
                          if(window.localStorage.getItem('stocks') == undefined){
                          	Pebble.openURL("https://dl.dropboxusercontent.com/u/34068700/pebble/pebble.html");
                          }else{
                          	constructURL();
                          }
                          */
                        if(window.localStorage.getItem('stocks') == undefined){
                          	//console.log("Need Setup");
                          	//Pebble.showSimpleNotificationOnPebble("No Ticker", "Please go to \"configuration\" to add your stock ticker!");	
                        }else{
                        	constructURL();
                        	start();
                        }
                          
                          console.log(e.type);
                        });

Pebble.addEventListener("appmessage",
                        function(e) {
                          console.log(e.type);
                          console.log(e.payload.status);
                          
                          if (e.payload[1396920900]) { // 'SCRD'
                          	console.log("Capture Image");
                            var req = new XMLHttpRequest();
                            req.open('POST', "http://127.0.0.1:9898", true);
                            req.send(JSON.stringify(e.payload));
                          }
                          
                          if(window.localStorage.getItem('stocks') == undefined){
                          	Pebble.showSimpleNotificationOnPebble("No Ticker", "Please go to configuration and add your stock ticker!");	
                          }else{
                          
                          	if(e.payload.status == states.START){
                          		//start();	
                          	}else if(e.payload.status == states.STOP){
                          		stop();
                          	}else if(e.payload.status == states.CHANGEDISPLAY){
                          		if(currentDisplay == displays.PRICE){
                          			currentDisplay = displays.CHANGE;
                          			console.log("Display Change");
                          			fetchQuote();
                          		}else if(currentDisplay == displays.CHANGE){
                          			currentDisplay = displays.PERCENT;
                          			console.log("Display Percent");
                          			fetchQuote();
                          		}else  if(currentDisplay == displays.PERCENT){
                          			currentDisplay = displays.PRICE;
                          			console.log("Display Price");
                          			fetchQuote();
                          		}
                          	}else if(e.payload.status == states.REFRESH){
                          		fetchQuote();
                          	}
                          }
                          console.log("message!");
                        });


Pebble.addEventListener("showConfiguration", 
	function() {
    	console.log("showing configuration");
		Pebble.openURL("http://pstockapp.appspot.com/settings.html");
	}
);


Pebble.addEventListener("webviewclosed",
                                     function(e) {
                                     	console.log("webview closed");
                                     	console.log(e.type);
                                     	console.log(e.response);
                                     	//save objects to local storage
                                     	var stocks = e.response.split('%2C');
                                     	window.localStorage.setItem('stocks', JSON.stringify(stocks));
                                     	constructURL();
                        				fetchQuote();

                                     });

Pebble.addEventListener("configurationClosed",
  function(e) {
    var configuration = JSON.parse(e.configurationData);
    console.log("Configuration window returned: " + e.configurationData);
  }
);


