/**
 * Avinesh P.V.S
 * UKP Labs
 *
 * Copyright (c) 2018 Avinesh P.V.S. All rights reserved.
 *
 * Do not use without prior consent by the copyright holder.
 *
 **/

import React, {PropTypes} from 'react';
import {Link} from 'react-router';
import {
    LinkContainer,
    IndexLinkContainer
} from 'react-router-bootstrap';
import {
    Navbar,
    Nav,
    NavItem,
    NavDropdown,
    MenuItem
} from 'react-bootstrap';
import FontAwesome from 'react-fontawesome';

import {connect} from 'react-redux';
import {bindActionCreators} from 'redux';

import routeConfig from '../../config/routes';
import * as UserActions from '../../controllers/user/actions';
import './header.less';
/**
 *
 * @param props
 * @constructor
 */
const Header = (props) => {
    const {store, actions} = props;

    let username = store.users.user.id;

    return (
            <Navbar componentClass="header" className="header" fixedTop>
                <Navbar.Header>
                    <Navbar.Brand>
						{store.app.title}
                        {/*<Link to={routeConfig.root}> </Link>*/}
                    </Navbar.Brand>
                    <Navbar.Toggle/>
                </Navbar.Header>
                {/*
                <Navbar.Collapse>
                    {<Nav>
                        <IndexLinkContainer to={routeConfig.data}>
                            <NavItem eventKey={2}><FontAwesome name="folder" fixedWidth/>&nbsp;Files</NavItem>
                        </IndexLinkContainer>
                    </Nav>
                    <Nav>
                        <IndexLinkContainer to={routeConfig.assignments}>
                            <NavItem eventKey={2}><FontAwesome name="shopping-basket" fixedWidth/>&nbsp;
                                Assignments</NavItem>
                        </IndexLinkContainer>
                    </Nav>}
                    <Nav pullRight>
                        <IndexLinkContainer to={routeConfig.help}>
                            <NavItem eventKey={3}><FontAwesome name="question-circle-o" fixedWidth/>&nbsp;Help</NavItem>
                        </IndexLinkContainer>
                    </Nav>
                </Navbar.Collapse>
                */}
            </Navbar>);
};


/**
 * Maps values from the application state to properties
 * of the container component.
 * @param state {State} - the current application state
 */
const mapStateToProps = ({users, app} = state) => ({
    store: {
        users,
        app
    }
});

/**
 * Maps action dispatchers to properties of the container
 * component.
 *
 * @param dispatch {Dispatch} - the stores dispatch function.
 */
const mapDispatchToProps = (dispatch) => ({
    actions: bindActionCreators(Object.assign({}, UserActions), dispatch)
});

export default connect(mapStateToProps, mapDispatchToProps)(Header);


/** WEBPACK FOOTER **
 ** ./src/main/javascript/containers/Header/index.jsx
 **/
