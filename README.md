# P2P_File_Transfer
## Distributed Programming Project #1
<p>Peer to peer file transfer using a centralizing server which facillitates the completion of the tasks required by the peers.</p>

<p>The network is made up of a maximum number of peers and a centralizing server. 
Each peer runs 2 processes in parallel, the client process which can request files from the network and the server process which runs in background and serves requests from peers willing to download a file. 
</p>
<p>
The centralizing server concurrently serves requests from peers by giving them further information on how to make the connections to other peers and pull segments of the requested file. 
</p>


The user will interface this application through the client process of a peer which will allow it issue one of the 2 requests: 
- DOWNLOAD FILE   - download a file identified by an md5 hash 
- VIEW FILES      - see the list of files that can be downloaded from the network. 
