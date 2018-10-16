import React from 'react';
import './styles.less';
import videojs from 'video.js';

export default class Video extends React.Component {

    constructor(...args) {
        super(...args);
    }

    componentDidMount() {
        // instantiate video.js

        this.player = videojs(this.videoContainer, this.props, function onPlayerReady() {
            console.log('onPlayerReady', this)
        });
        if(this.props.poster !== undefined) {

        }
        this.player.addClass('vjs-cascade');
    }

    componentWillUnmount() {
        if (this.player) {
            this.player.dispose();
        }
    }

    render() {
        const src = this.props.src;
        const type = this.props.type || "video/mp4";


        return (<div data-vjs-player>
            <video controls className='video-js vjs-fluid'
                   ref={el => this.videoContainer = el}>
                <source src={this.props.src} type={type}/>
                Your browser does not support the video tag.
            </video>
        </div>);
    }
};
