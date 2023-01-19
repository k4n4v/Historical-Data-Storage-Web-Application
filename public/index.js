// Put all onload AJAX calls here, and event listeners
jQuery(document).ready(function() {

    document.getElementById("gpxDisplaying").innerHTML = "Displaying: no file";

/************** 
 * Populate file log
 * ***********/ 
    function updateFileLog(){
        
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/updateFileLog',   //The server endpoint we are connecting to
    
            success: function (data) {
    
                //console.log(data);
                //console.log(data.info[0]);
                //console.log(data.names);
    
                var table = document.getElementById("table_file");
                var dropdown_gpxView = document.getElementById("dropdown_gpxView");
                var dropdown_component = document.getElementById("dropdown_otherData");
                var button_rename = document.getElementById("button_rename");
                var button_showOther = document.getElementById("button_showOther");
                var button_addRoute = document.getElementById("button_addRoute");
                var button_findPath = document.getElementById("button_findPath");
                var length;
                var row;
                var temp;
    
                //checks for and adds the files on server
                for( var i=0; i<data.info.length; i++ ){
                    
                    temp = JSON.parse(data.info[i]);
                    
                    length = table.rows.length;
                    
                    row = table.insertRow(length);
                    row.insertCell(0).innerHTML = '<a href="' + data.names[i] + '" download>' + data.names[i] + '</a>';
                    row.insertCell(1).innerHTML = temp.version;
                    row.insertCell(2).innerHTML = temp.creator;
                    row.insertCell(3).innerHTML = temp.numWaypoints;
                    row.insertCell(4).innerHTML = temp.numRoutes;
                    row.insertCell(5).innerHTML = temp.numTracks;
    
                    //add file name to gpx view
                    var option = document.createElement("option");
                    option.text = data.names[i];
                    dropdown_gpxView.add(option);
    
                    var option2 = document.createElement("option");
                    option2.text = data.names[i];
                    dropdown_addRoute.add(option2);
                }
    
                var totalRowCount = table.rows.length;
                //console.log(totalRowCount);
    
                //if there are no files on the sever
                if( totalRowCount == 1 ){
    
                    var length = table.rows.length;
                    var row = table.insertRow(length);
                    var cell1 = row.insertCell(0);
                    cell1.innerHTML = "No Files";
    
                    var option = document.createElement("option");
                    option.text = "No Files";
                    dropdown_gpxView.remove(0);
                    dropdown_gpxView.add(option);
                    dropdown_gpxView.disabled = true;
    
                    dropdown_component.add(option);
                    dropdown_component.disabled = true;
    
                    dropdown_addRoute.add(option);
                    dropdown_addRoute.disabled = true;
                    button_addRoute.disabled = true;
                    button_addWpt.disabled = true;
    
                    button_showOther.disabled = true;
                    button_rename.disabled = true;

                    button_addRoute.disabled = true;
                    button_findPath.disabled = true;
    
                    console.log("No files on server");
                }
    
            },
            fail: function(error) {
                console.log(error); 
            }
        });    
    }

    updateFileLog();



/************** 
 * GPX view table
 * ***********/ 
    $("#dropdown_gpxView").on("change", function(){
        var option = document.getElementById("dropdown_gpxView").value;
        document.getElementById("gpxDisplaying").innerHTML = "Displaying: " + option;
        //console.log(option);
        var dropdown_otherData = document.getElementById("dropdown_otherData");
        
        if( !(option == "Select a File") ){
            jQuery.ajax({
                type: 'get',            //Request type
                dataType: 'json',       //Data type - we will use JSON for almost everything 
                url: '/updateGPXView',   //The server endpoint we are connecting to
                data: {
                    file: option,
                },
                success: function (data) {
    
                    $("#dropdown_otherData").find("option:gt(0)").remove(); //reset the table when changing select
                    $("#table_gpx").find("tr:gt(0)").remove(); //reset the dropdown when changing select
    
                    var table = document.getElementById("table_gpx");
                    //routes
                    for( var i=0; i<data.route.length; i++ ){
    
                        length = table.rows.length;
                        
                        row = table.insertRow(length);
                        row.insertCell(0).innerHTML = "Route "+(i+1);
                        if( data.route[i].name == "None"){
                            row.insertCell(1).innerHTML = "";
                        }else{
                            row.insertCell(1).innerHTML = data.route[i].name;
                        }
                        row.insertCell(2).innerHTML = data.route[i].numPoints;
                        row.insertCell(3).innerHTML = data.route[i].len;
                        row.insertCell(4).innerHTML = data.route[i].loop;
    
                        //add file name to component drop down
                        var option = document.createElement("option");
                        option.text = "Route "+(i+1);
                        dropdown_otherData.add(option);
                    }
    
                    //tracks
                    for( var i=0; i<data.track.length; i++ ){
    
                        length = table.rows.length;
                        
                        row = table.insertRow(length);
                        row.insertCell(0).innerHTML = "Track "+(i+1);
                        if( data.track[i].name == "None"){
                            row.insertCell(1).innerHTML = "";
                        }else{
                            row.insertCell(1).innerHTML = data.track[i].name;
                        }
                        row.insertCell(2).innerHTML = data.track[i].numPoints;
                        row.insertCell(3).innerHTML = data.track[i].len;
                        row.insertCell(4).innerHTML = data.track[i].loop;
    
                        //add file name to component drop down
                        var option = document.createElement("option");
                        option.text = "Track "+(i+1);
                        dropdown_otherData.add(option);
                    }
                },
                fail: function(error) {
                    console.log(error); 
                }
            });    
        }
    });


/************** 
 * Show other data 
 * ***********/ 
    $("#button_showOther").on("click", function(){

        var option = document.getElementById("dropdown_otherData").value;
        var file = document.getElementById("dropdown_gpxView").value;
        var component;

        if( option !== "Components" && file !== "Select a File" ){

            if( option.includes("Track") ){
                component = "Track"; 
            
            }else if( option.includes("Route") ){
                component = "Route";
            }
    
            var num = option.replace( /^\D+/g, '');
    
            jQuery.ajax({
                type: 'get',            //Request type
                dataType: 'json',       //Data type - we will use JSON for almost everything 
                url: '/updateOtherData',   //The server endpoint we are connecting to
                data: {
                    file: file,
                    component: component,
                    num: num,
                },
                success: function (data) {
    
                    console.log(data);
    
                    if( data.length == 0 ){
                        alert("This componenet contains no other data.");
                    }else{
    
                        for( var i=0; i<data.length; i++ ){
                            alert(data[i].name + ": " + data[i].value);
                        }
                    }
    
                },
                fail: function(error) {
                    console.log(error); 
                }
            }); 
    
        }else{
            alert("Please select a component");
            console.log("Error: Component not selected");
        }
    });

/************** 
 * Rename component 
 * ***********/ 
 $("#button_rename").on("click", function(e){
        
        var option = document.getElementById("dropdown_otherData").value;
        var file = document.getElementById("dropdown_gpxView").value;
        var component;

        if( option !== "Components" && file !== "Select a File" ){
            if( option.includes("Track") ){
                component = "Track"; 
            
            }else if( option.includes("Route") ){
                component = "Route";
            }
    
            var num = option.replace( /^\D+/g, '');
    
            var rename = document.getElementById("rename").value;
            
            e.preventDefault();
    
            jQuery.ajax({
                type: 'get',            //Request type
                dataType: 'json',       //Data type - we will use JSON for almost everything 
                url: '/rename',   //The server endpoint we are connecting to
                data: {
                    file: file,
                    component: component,
                    num: num,
                    rename: rename,
                },
                success: function (data) {
                    alert( option + " has been renamed to " + rename);
                },
                fail: function(error) {
                    console.log(error); 
                }
            });     
        }else{
            alert("Please select a component");
            console.log("Error: Component not selected");
        }
    });


/************** 
 * Add route
 * ***********/ 
$("#button_addWpt").on("click", function(){

    var form = document.getElementById("form_addRoute");

    var label1 = document.createElement("label");
    label1.innerHTML = "Latitude ";
    form.appendChild(label1);

    var input1 = document.createElement("input");
    input1.type = "number";
    input1.placeholder = "Enter latitude";
    input1.min = "-90";
    input1.max = "90";
    input1.required = true;
    form.appendChild(input1);

    var label2 = document.createElement("label");
    label2.innerHTML = "Longitude ";
    form.appendChild(label2);

    var input2 = document.createElement("input");
    input2.type = "number";
    input2.placeholder = "Enter longitude";
    input2.min = "-180";
    input2.max = "180";
    input2.required = true;

    form.appendChild(input2);
    
    var space = document.createElement("br");
    form.appendChild(space);
});

 $("#button_addRoute").on("click", function(e){
        
    var routeName = document.getElementById("routeName").value;
    var file = document.getElementById("dropdown_addRoute").value;
    
    //console.log(routeName);
    //console.log(file);

    e.preventDefault();

        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/addRoute',   //The server endpoint we are connecting to
            data: {
                routeName: routeName,
                file: file,
            },
            success: function (data) {
                //console.log(data);
                alert("Route added to " + fileName);
                
            },
            fail: function(error) {
                console.log(error); 
            }
        }); 
});


/************** 
 * Create GPX
 * ***********/ 
$("#button_createGPX").on("click", function(e){

    var filename = document.getElementById("gpxName").value;
    var num = 1.1;
    var creator = document.getElementById("gpxCreator").value;
    
    //console.log(filename);
    //console.log(num);
    //console.log(creator);

    if( filename.search(".gpx") == -1 ){
        //console.log(filename);

        filename = filename + ".gpx";
    }

    //console.log(filename);

    e.preventDefault();

    if (/[^a-zA-Z]/.test(creator) || creator == ""){
        console.log("Error: Invalid creator")
        alert("Please enter a vaild creator (must be all letters)");

    }else{
        var doc = {
            version: num,
            creator: creator,
        }
    
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/createGPX',   //The server endpoint we are connecting to
            data: {
                filename: filename,
                doc: doc,
            },
            success: function (data) {
                //console.log(data);
                alert("File with same name already exists");
            },
            fail: function(error) {
                console.log(error); 
            }
        }); 
    }
});


/************** 
 * Find path
 * ***********/ 
 $("#button_findPath").on("click", function(e){
        
    var startLat = document.getElementById("startLat").value;
    var startLon = document.getElementById("startLon").value;
    var endLat = document.getElementById("endLat").value;
    var endLon = document.getElementById("endLon").value;
    var delta = document.getElementById("delta").value;

    //console.log(startLat);
    //console.log(startLon);
    //console.log(endLat);
    //console.log(endLon);
    //console.log(delta);

    e.preventDefault();
    if( startLat && startLon && endLat && endLon && delta ){
        
        jQuery.ajax({
            type: 'get',            //Request type
            dataType: 'json',       //Data type - we will use JSON for almost everything 
            url: '/findPath',   //The server endpoint we are connecting to
            data: {
                startLat: startLat,
                startLon: startLon,
                endLat: endLat,
                endLon: endLon,
                delta: delta,
            },
            success: function (data) {
                console.log(data);
                $("#table_path").find("tr:gt(0)").remove(); //reset the dropdown when changing select

                var table = document.getElementById("table_path");
                //routes
                for( var i=0; i<data.route.length; i++ ){

                    length = table.rows.length;
                    
                    row = table.insertRow(length);
                    row.insertCell(0).innerHTML = "Route "+(i+1);
                    if( data.route[i].name == "None"){
                        row.insertCell(1).innerHTML = "";
                    }else{
                        row.insertCell(1).innerHTML = data.route[i].name;
                    }
                    row.insertCell(2).innerHTML = data.route[i].numPoints;
                    row.insertCell(3).innerHTML = data.route[i].len;
                    row.insertCell(4).innerHTML = data.route[i].loop;
                }

                //tracks
                for( var i=0; i<data.track.length; i++ ){

                    length = table.rows.length;
                    
                    row = table.insertRow(length);
                    row.insertCell(0).innerHTML = "Track "+(i+1);
                    if( data.track[i].name == "None"){
                        row.insertCell(1).innerHTML = "";
                    }else{
                        row.insertCell(1).innerHTML = data.track[i].name;
                    }
                    row.insertCell(2).innerHTML = data.track[i].numPoints;
                    row.insertCell(3).innerHTML = data.track[i].len;
                    row.insertCell(4).innerHTML = data.track[i].loop;
                }

            },
            fail: function(error) {
                console.log(error); 
            }
        }); 

    }else{
        console.log("Error: find path entry is invalid");
        alert("Error: All entires must be filled with decimal values");
    }
});





});
