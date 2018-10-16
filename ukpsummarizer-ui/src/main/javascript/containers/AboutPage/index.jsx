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
import {
    Grid,
    Row,
    Col,
    PageHeader
} from 'react-bootstrap';

export const AboutPage = () => (
    <Grid>
        <Row>
            <Col xs={12}>
                <PageHeader>About</PageHeader>
                <address>
					<strong>UKP Lab</strong><br/>
                    <a href="https://www.ukp.tu-darmstadt.de/ukp-home/">https://www.ukp.tu-darmstadt.de/ukp-home/</a>
                </address>

            </Col>
        </Row>
    </Grid>
);

/**
 * Maps values from the application state to properties
 * of the container component.
 * @param state {State} - the current application state
 */
const mapStateToProps = (state) => ({
});

/**
 * Maps action dispatchers to properties of the container
 * component.
 *
 * @param dispatch {Dispatch} - the stores dispatch function.
 */
const mapDispatchToProps = (dispatch) => ({
});

export default connect(mapStateToProps, mapDispatchToProps)(AboutPage);
