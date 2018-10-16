"use strict";
/**
 * Hendrik Lücke-Tieke
 * dataexpedition.net
 *
 * Copyright (c) 2017 Hendrik Lücke-Tieke. All rights reserved.
 *
 * Do not use without prior consent by the copyright holder.
 *
 **/
import React from 'react';
import {connect} from 'react-redux';
import NotFoundPanel from '../../components/NotFoundPanel'
import {Grid, Row, Col, Jumbotron} from 'react-bootstrap';

class NotFoundPage extends React.Component {
    componentDidMount() {
        window.scrollTo(0, 0);
    }

    render() {
        const {location} = this.props;

        return (
            <Grid>
                <Row>
                    <Col>
                        <NotFoundPanel location={location}/>
                    </Col>
                </Row>
            </Grid>
        );
    }
}

const mapStateToProps = (state) => ({
    location: state.routing.locationBeforeTransitions.pathname
});

export default connect(mapStateToProps)(NotFoundPage);
