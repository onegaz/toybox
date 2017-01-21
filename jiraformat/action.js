var fs = require('fs');
var app = require('electron').remote;
var dialog = app.dialog; 

function currentTimeStr() {
    var currentdate = new Date(); 
    var datetime = currentdate.getDate() + "/"
        + (currentdate.getMonth()+1)  + "/" 
        + currentdate.getFullYear() + " "  
        + currentdate.getHours() + ":"  
        + currentdate.getMinutes() + ":" 
        + currentdate.getSeconds();
    return datetime;
}

function jira_format(inputstr) {
    
    var outputstr = "";
    for(var i=0, len = inputstr.length; i<len; i++) {
        var onechar = inputstr.charAt(i);
        switch(onechar) {
            case '[':
            case '|':
            case '-':
            case '{':
            case '\\':
            case '#':
            case '*':
            outputstr += '\\' + onechar;
            break;
            default:
            outputstr += onechar;
            break;
        }
    }
    return outputstr;
}

document.getElementById('btn_jiraformat').addEventListener('click', function() {
    console.log(currentTimeStr() + " format button clicked");
    document.getElementById("outputtext").value += jira_format(document.getElementById("inputtext").value);
}, false);

document.getElementById('btn_clear').addEventListener('click', function() {
    console.log(currentTimeStr() + " clear button clicked");
    document.getElementById("outputtext").value = currentTimeStr();
    document.getElementById("inputtext").value = "";
}, false);