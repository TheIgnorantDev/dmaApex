//create express app
var express = require("express");
var app = express();
var bodyParser = require("body-parser");
app.use(bodyParser.urlencoded({ extended: false }));
app.use(bodyParser.json());
app.use(express.static(__dirname + '/src'));

app.get("/", function (req, res) {
    //send the src/app.html file to the client
    res.sendFile(__dirname + "/src/app.html");
});


app.listen(3000, function () {
    console.log("Server running on port 3000");
});