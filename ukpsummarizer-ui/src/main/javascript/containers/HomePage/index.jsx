/**
 * Hendrik Lücke-Tieke
 * dataexpedition.net
 *
 * Copyright (c) 2017 Hendrik Lücke-Tieke. All rights reserved.
 *
 * Do not use without prior consent by the copyright holder.
 *
 **/
// mvc stuff
import {connect} from 'react-redux';
import routeConfig from '../../config/routes';
import {LinkContainer} from "react-router-bootstrap";


// ui stuff
import './style.less';
import {Link} from 'react-router'
import React from 'react';
import {
    Button,
    Grid,
    Row,
    Col,
    Jumbotron,
    Well,
    Panel, Collapse
} from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';
import HoverPanel from '../../components/HoverPanel';

class HomePage extends React.Component {

    state = {
        documentsVisible: false
    };

    componentDidMount() {
        window.scrollTo(0, 0);
    }

    render() {

        return (<Grid>
            <Row>
                <Col xs={12}>
                        <h1>Sherlock</h1>
                        <p>Computer Aided Summarization to Combat Accelerated Data Exploration</p>
                </Col>
            </Row>

            <Row>
                <Col md={12}>
                    <p className="lead">
                    Collect a summary text for a given query by iteratively marking relavant information in the summary text for a specific query, until your desired summary is reached.
                    </p>
                </Col>
            </Row>
            <Well> 
                <Row>
                    <Col md={12}>
                        <b> Example Topic: </b> Art and music in public schools 
                    </Col>
                </Row>
                <Row>
                    <Col md={12}>
                    <b> Query: </b> Describe the state of teaching art and music in public schools around the world. Indicate problems, progress and failures.
                    </Col>
                </Row>
                <Row>
                    <Col md={12}>
                    <b> Summary: </b> China, Cuba and the United States are countries trying to improve music and arts education in the public schools.
                             Despite disagreement as to the importance of this instruction to other aspects of the students' development, its value in its own right is accepted.
                             It is also generally agreed that much demands to be done.
                             In China music and drama weekends supplement the school curriculum, special schools train rural schoolteachers in music and the arts and schools are being equipped with necessary facilities for music and arts instruction.
                             In Cuba music education is emphasized but the instruments are old and in need of repair.
                             In the United States there is widespread recognition of damage done by 20-30 years of elimination or reduction of music and arts classes due to budget cuts or increased emphasis on core subjects to satisfy testing requirements.
                             Various approaches are being tried as remedies.
                             U.S. Secretary of Education Riley has said, "...most American children are infrequently or never given serious instruction or performance opportunities in music, the arts or theater. That's wrong".
                             Across the country efforts to compensate for cutbacks include: raising private funds; arranging assistance to the public schools from cultural, educational and non-profit organizations; development of digital techniques for music  and the arts; offering free after-school and summer programs; introducing low-cost equipment; blending the arts and music into core subjects; and including music and arts as college admission requirements.
                    </Col>
                </Row> 
            </Well>
            <Row>
                <Col sm={12} xs={12}>
                        <Link to={routeConfig.assignments}>
                            <Button bsStyle='primary'> Continue </Button>
                        </Link>
                </Col>

            </Row>
        </Grid>);
    }
}

/**
 * Maps values from the application state to properties
 * of the container component.
 * @param state {State} - the current application state
 */
const mapStateToProps = (state) => ({});

/**
 * Maps action dispatchers to properties of the container
 * component.
 *
 * @param dispatch {Dispatch} - the stores dispatch function.
 */
const mapDispatchToProps = (dispatch) => ({});

export default connect(mapStateToProps, mapDispatchToProps)(HomePage);
