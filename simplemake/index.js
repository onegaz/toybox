var fs = require('fs');
var app = require('electron').remote;
var dialog = app.dialog;

function enumerateSourceFiles() {
    var path = require( 'path' );
    var process = require( "process" );
    var currentDir = "./";                        
    fs.readdir( currentDir, function( err, files ) {
        if( err ) {
            console.error( "Could not list the directory.", err );
            process.exit( 1 );
        } 
        files.forEach( function( file, index ) {
                // Make one pass and make the file complete
                var fromPath = path.join( currentDir, file );                                
                fs.stat( fromPath, function( error, stat ) {
                    if( error ) {
                        console.error( "Error stating file.", error );
                        return;
                    }

                    if( stat.isFile() ) {
                        var match = file.match(/.*[.]+(cxx|cpp|cc)/i);
                        if (match !== null)
                        {
                            document.getElementById("cpp-src").value += file + " ";
                            console.log( "'%s' is C/C++ source file.", fromPath );
                        } else {
                            console.log( "'%s' is not C/C++ source file.", fromPath );
                        }
                            
                    }                                        
                    else if( stat.isDirectory() )
                        console.log( "'%s' is a directory.", fromPath );                                   
                } );
        } );
    } );   
}

// document.getElementById("cpp-src").value += enumerateSourceFiles(); // not working because some operation in enumerateSourceFiles is async
enumerateSourceFiles();

function singleline(mulitilines, prefix) {
    var lines = mulitilines.split(/\r?\n/);                
    console.log('number of lines %d: ', lines.length);
    var output = "";
    for (var i = 0, len = lines.length; i<len; i++){
        if(lines[i].length<=1)
            continue;
        output += prefix+lines[i];
        if(i < len-1)
            output += " ";                    
    }
    return output;
}

function createMakefile() {
    var src = singleline(document.getElementById("cpp-src").value, "");
    var includes = singleline(document.getElementById("includes").value, "-I");
    var libs = singleline(document.getElementById("libs").value, "-L");
    var cxxflags = singleline(document.getElementById("cxxflags").value, "");
    var ldflags = singleline(document.getElementById("ldflags").value, "");
    var target = document.getElementById("target").value;
    var content = "all: " + target + "\n\n"
    content += "INCLUDES="+includes + "\n\n";
    content += "LIBPATH="+ libs + "\n\n";
    content += "CXXFLAGS="+ cxxflags + "\n\n";
    content += "LDFLAGS="+ ldflags + "\n\n";
    content += "clean: \n";
    content += "\trm " + target + "\n\n";
    content += target + ": " + src + "\n";
    content += "\t" + "g++ -o " + target + " $(CXXFLAGS) $(INCLUDES) $(LDFLAGS) $(LIBPATH) " + src;
    fileName = "Makefile";
    fs.writeFile(fileName, content, function (err) {
        if(err){
            console.log("An error ocurred creating Makefile "+ err.message)
            document.getElementById("buildoutput").value += currentTimeStr() + " An error ocurred creating Makefile "+ err.message + "\n";
        }                    
        console.log("The Makefile has been succesfully saved");
        document.getElementById("buildoutput").value += currentTimeStr() + " The Makefile has been succesfully saved\n";
    });
}

document.getElementById('select-cpp-src').addEventListener('click',function(){
    // allow user select source file name, append to document.getElementById("cpp-src").value
    dialog.showOpenDialog(function (fileNames) {
    if (fileNames === undefined) return;
    var fileName = fileNames[0];
    document.getElementById("cpp-src").value += " " + fileName;
    }); 
},false);

function select_path_for(elementid) {
    var path = dialog.showOpenDialog({
        properties: ['openDirectory']
    });
    if (path === undefined) return;
    document.getElementById(elementid).value += path + "\n";
}

document.getElementById('select-includes').addEventListener('click',function(){
    // allow user select includes path, append to document.getElementById("includes").value
    select_path_for("includes");
},false);

document.getElementById('select-libs').addEventListener('click',function(){
    // allow user select link path, append to document.getElementById("libs").value
    select_path_for("libs");
},false);            

document.getElementById('Makefile').addEventListener('click',function(){
    createMakefile();
},false);

function appendoutput(elementid, newmessage, max_output_lines) { // limit the number of lines to max_output_lines. if exceeds, remove some lines so max_output_lines/2 lines left
    var stream = document.getElementById(elementid).value + newmessage;
    var lines = stream.split(/\r?\n/);
    var numberToRemove = lines.length - max_output_lines/2;
    console.log('numberToRemove %d: ', numberToRemove);
    if(lines.length >max_output_lines ) {
        removedlines = lines.splice(0, numberToRemove);
        var output = "";
        for (var i = 0, len = lines.length; i<len; i++){
            if(i < len-1)
            output += lines[i] +"\n";
            else
            output += lines[i];
        }
        // document.getElementById("buildoutput").value += `${stdout}`;  
        document.getElementById(elementid).value = output;                         
    } else {
        document.getElementById(elementid).value += newmessage;
    }                    
}

function buildfunc1() {
    fs.stat('Makefile', function(err, stat) {
        if(err == null) {
            // console.log('File exists');
        } else if(err.code == 'ENOENT') {
            // file does not exist
            console.log('Generate Makefile');
            createMakefile();
        } else {
            console.log('Error when checking Makefile status: ', err.code);
        }
    });
    const exec = require('child_process').exec;
    var cmdline;                
    cmdline ='make' ;
    //console.log(`refreshTimer is working ` + datetime + ' ' + cmdline);
    document.getElementById("buildoutput").value += currentTimeStr() + ' ' + cmdline +"\n";
    exec(cmdline, (error, stdout, stderr) => {
        if (error) {
            console.error(`exec error: ${error}`);
            document.getElementById("buildoutput").value += `${error}`;
            return;
        }
        document.getElementById("buildoutput").value += `${stdout}`;
        
    }); 
}
function buildfunc2() {
    fs.stat('Makefile', function(err, stat) {
        if(err == null) {
            // console.log('File exists');
        } else if(err.code == 'ENOENT') {
            // file does not exist
            console.log('Generate Makefile');
            createMakefile();
        } else {
            console.log('Error when checking Makefile status: ', err.code);
        }
    });
    const exec = require('child_process').exec;
    var cmdline ='make' ;
    var max_output_lines = 4;
    const spawn = require('child_process').spawn;
    var cmdoutput = spawn(cmdline); 
    // add a 'data' event listener for the spawn instance
    cmdoutput.stdout.on('data', function(data) { appendoutput("buildoutput", data, max_output_lines); });
    // add an 'end' event listener to close the writeable stream
    cmdoutput.stdout.on('end', function(data) {
        appendoutput("buildoutput", "done", max_output_lines);
    });
    // when the spawn child process exits, check if there were any errors and close the writeable stream
    cmdoutput.on('exit', function(code) {
        if (code != 0) {
            appendoutput("buildoutput", 'Failed: ' + code, max_output_lines);
            console.log('Failed: ' + code);
        }
    });
}

document.getElementById('build').addEventListener('click',function(){
    buildfunc2();
    return;
},false);

document.getElementById('clean').addEventListener('click',function(){
    const exec = require('child_process').exec;
    var cmdline;                
    cmdline ='make clean' ;
    //console.log(`refreshTimer is working ` + datetime + ' ' + cmdline);
    document.getElementById("buildoutput").value += currentTimeStr() + ' ' + cmdline +"\n";
    exec(cmdline, (error, stdout, stderr) => {
        if (error) {
            console.error(`exec error: ${error}`);
            document.getElementById("buildoutput").value += `${error}`;
            return;
        }
        document.getElementById("buildoutput").value += `${stdout}`;                    
    });                 
},false);

document.getElementById('clear').addEventListener('click',function(){
    document.getElementById("buildoutput").value = "";             
},false);

function readFile(filepath) {
    fs.readFile(filepath, 'utf-8', function (err, data) {
        if(err){
            alert("An error ocurred reading the file :" + err.message);
            return;
        }
        
        document.getElementById("content-editor").value = data;
    });
}

Date.prototype.yyyymmdd = function() {
  var mm = this.getMonth() + 1; // getMonth() is zero-based
  var dd = this.getDate();
  return [this.getFullYear(),
          (mm>9 ? '' : '0') + mm,
          (dd>9 ? '' : '0') + dd
         ].join('');
};

function currentTimeStr() {
    var currentdate = new Date(); 
    var datetime = currentdate.yyyymmdd() + " "
        + currentdate.getHours() + ":"  
        + currentdate.getMinutes() + ":" 
        + currentdate.getSeconds();
    return datetime;
}
