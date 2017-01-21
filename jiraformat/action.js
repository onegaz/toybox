var fs = require('fs');
var app = require('electron').remote;
var dialog = app.dialog; 

var DEBUG = false;
if(!DEBUG){
    if(!window.console) window.console = {};
    var methods = ["log", "debug", "warn", "info"];
    for(var i=0;i<methods.length;i++){
        console[methods[i]] = function(){};
    }
}

function num2str(num) {
    return (num>9?"":"0") + num;
}

Date.prototype.yyyymmdd = function() {
  var mm = this.getMonth() + 1; // getMonth() is zero-based
  var dd = this.getDate();
  return [this.getFullYear(),
          num2str(this.getMonth() + 1),
          num2str(this.getDate())
         ].join('');
};

function date2str(currentdate) {
    var datetime = currentdate.yyyymmdd() + " "
    + num2str(currentdate.getHours()) + ":"  
    + num2str(currentdate.getMinutes()) + ":" 
    + num2str(currentdate.getSeconds());
    return datetime;
}

function currentTimeStr() {
    var currentdate = new Date(); 
    return date2str(currentdate);
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

function sort_email_discussion(inputsrc) {
    var lines = inputsrc.split(/\r?\n/);    
    var fromdates = [] // start line number of each discussion
    var original_discussion_date_line = new Map(); // key: email sent datetime, value: start line number

    for (var i=0, len=lines.length; i<len; i++) {
        var match = lines[i].match(/^From: .*/i); // check From: 
        if (match !== null)
        {
            if(i+1<len) { // check Sent: 
                var sentmatch = lines[i+1].match(/^Sent: .* (AM|PM)/i);
                if (sentmatch !== null) {
                    fromdates.push(i);
                    console.log("%s", lines[i+1]);
                    var sentdate = new Date(lines[i+1].substr("Sent: ".length));
                    console.log("Sent date is %s", date2str(sentdate));
                    original_discussion_date_line.set( date2str(sentdate), i );
                }
            }            
        } 
        var phonefrom = lines[i].match(/.*On .*, at .* (AM|PM),.* wrote:/i);
        if (phonefrom !== null) {
            fromdates.push(i);
            console.log("%s", lines[i]);
            var re = /.*On (.*), at (.* (AM|PM))(.*)/;
            var datestr = lines[i].replace(re, '$1 $2');
            var sentdate = new Date(datestr);
            console.log("parsed date string is %s, recognize as %s", datestr, date2str(sentdate));
            original_discussion_date_line.set( date2str(sentdate), i );
        }          
    }    
// From: First, Last 
// Sent: Friday, January 20, 2017 6:32 PM      
// On Jan 17, 2017, at 3:32 PM, First, Last <First.Last@abcd.com> wrote:
    if(original_discussion_date_line.size<2) {
        console.log("No need to reorder since there is only %d date", original_discussion_date_line.size)
        return inputsrc;
    }
        
    var outputstr = "";
    var mapAsc = new Map([...original_discussion_date_line.entries()].sort());
    mapAsc.forEach( function(value, key, map) {        
        var discussion_end_line = lines.length;
        for(var i=0; i<fromdates.length; i++) {
            if(fromdates[i]==value) {
                if(i<fromdates.length-1)
                    discussion_end_line = fromdates[i+1];
            }
        }
        // output lines in this range [value, discussion_end_line)
        console.log("%s between line %d and line %d", key, value, discussion_end_line);
        for(var i=value; i<discussion_end_line; i++) 
            outputstr += lines[i]+"\n";
    });
    return outputstr;
}

document.getElementById('btn_jiraformat').addEventListener('click', function() {
    console.log(currentTimeStr() + " format button clicked");
	var sortedemail = sort_email_discussion(document.getElementById("inputtext").value);
    document.getElementById("outputtext").value = jira_format(sortedemail);
}, false);

document.getElementById('btn_clear').addEventListener('click', function() {
    console.log(currentTimeStr() + " clear button clicked");
    document.getElementById("outputtext").value = currentTimeStr();
    document.getElementById("inputtext").value = "";
}, false);