import React from 'react';
import Lightbox from 'react-image-lightbox';
import {Button, Image} from 'react-bootstrap';

export default class EnlargableImage extends React.Component {
    constructor(props) {
        super(props);

        this.state = {
            photoIndex: 0,
            isOpen: false
        };
    }

    render() {
        const {photoIndex, isOpen, } = this.state;
        const {src} = this.props;

        return (
            <div  onClick={() => this.setState({isOpen: true})}>
                <Image src={src} thumbnail responsive/>
                <p>Click to enlarge</p>
                {isOpen &&
                <Lightbox
                    mainSrc={src}
                    // nextSrc={src}
                    // prevSrc={src}

                    onCloseRequest={() => this.setState({isOpen: false})}
                    // onMovePrevRequest={() => this.setState({
                    //     photoIndex: (photoIndex + images.length - 1) % images.length,
                    // })}
                    // onMoveNextRequest={() => this.setState({
                    //     photoIndex: (photoIndex + 1) % images.length,
                    // })}
                />
                }
            </div>
        );
    }
}
