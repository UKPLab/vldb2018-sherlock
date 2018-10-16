import React from 'react';

const Video = ({src, type = "video/mp4", ...props}) => {
    let muted=false;
    if(props.hasOwnProperty("muted")) {
        if(props["muted"] === undefined || !!props.muted) {
            muted = "muted";
        } else {
            muted = undefined;
        }
    }


    return (<video controls style={{width: "100%", height: "100%"}}  muted={muted}>
        <source src={src} type={type}/>
        Your browser does not support the video tag.
    </video>);
};

export default Video;
