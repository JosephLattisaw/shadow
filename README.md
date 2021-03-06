# shadow
Core
- Command Line
    [x] Add command line options that allow program to run other programs
    [x] Add command line option for client mode
    [x] Add command line option to change client hostname
    [x] Add command line option to change client port
    [x] Add command line options for applications command line options
    [ ] Ability to check valid port numbers
    [ ] Ability to set no port (no port should be -1 option)
- Main Functionality
    [x] Run applications externally after being commanded
    [x] Run applications in separate threads created dynamically
    [x] Destroy applications via separate threads dynamically
- GUI (Generic)
    [x] Add Debugging GUI in order to Test Out GUI
    [ ] Add Play / Pause button to GUI
    [ ] Play / Pause buttons should be greyed out if in stop mode
    [ ] Play button should be greyed out in play mode
    [ ] Pause button should be greyed out in pause mode
- GUI (Apps and statuses)
    [x] Add apps and statuses to debugging GUI
    [x] Make apps and statues non editable
    [x] Add start and stop button to kill or start applications
    [x] update dynamically if application is running
    [x] Add colors for statuses
    [x] add ability to add command line arguments
    [x] remove focus from table items
    [x] remove ability to add command line options 
    [x] reorganize signals to be more capable
- Services
    [ ] Create Application that runs as service
    [ ] Make sure service keeps running even if crash
- Application Control
    [ ] Add TCP interface to allows play / pause functionality
    [ ] Create Control in order to allow play / pause functionality to programs
- Client
    [x] Add command line options to control if we are the client or the controller
    [x] Add command line options to set client hostname and port
    [x] Need to create a server which the client can talk to
    [x] Need to create a client which the server can talk to
    [x] Create keep alive for connecting between servers
    [x] Need to create protocol to share information over the client/server
    [x] Add ability for start / stop capability
    [x] Add ability for client to share application information with server
    [ ] always be ready to reconnect and kick last guy off the server/socket connection
    [ ] only one connection should be allowed
