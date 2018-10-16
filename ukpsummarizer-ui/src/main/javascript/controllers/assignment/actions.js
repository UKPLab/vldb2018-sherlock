import * as Actions from './constants';
import Rest from 'rest';
import template  from 'rest/interceptor/template';
import timeout from 'rest/interceptor/timeout';
import errorCode from 'rest/interceptor/errorCode';
import mime from 'rest/interceptor/mime';

import {loadSummary} from '../datasets/actions';
import * as SummaryActionTypes from '../summary/constants';
import * as UserAssignmentsActions from '../assignments/actions';
import * as AssignmentsActions from '../assignments/constants';
import {push} from 'react-router-redux';
import routerConfig from '../../config/routes';
import {loadAssignmentTemplate} from "../assignmenttemplates/actions";

export function createAssignment(topic) {
    return (dispatch, getState) => {

        const store = getState();
        // console.log("accepting assignments");
        // console.log(store.users.user.id);
        dispatch({
            type: Actions.USER_ACCEPT_ASSIGNMENT_REQUEST
        });
        dispatch({
            type: AssignmentsActions.LOAD_USER_ASSIGNMENTS_REQUEST
        });


        // if the topic is not loaded, then load it now:
        if (store.datasets.topic.path !== topic) {
            loadSummary(store.users.user.id, 0, topic);
        }
        // this.props.actions.loadSummary(this.props.store.users.user.id, 0, this.props.location.query.topic);


        let request = Rest.getDefaultClient()
            .wrap(mime)
            .wrap(timeout, {timeout: 10e6})
            .wrap(errorCode, {code: 500});
        request({
            method: "POST",
            path: `/assignments/create`,
            headers: {
                "u": store.users.user.id,
                "topic": topic,
                "Content-Type": "application/json"
            }
        }).then((success) => {
            // dispatch({
            //     type: SummaryActionTypes.RESET_STORE
            // });
            // dispatch({
            //     type: Actions.USER_ACCEPT_ASSIGNMENT_SUCCESS,
            //     payload: success.entity
            // });
            dispatch(UserAssignmentsActions.loadAssignments());
            dispatch(activateAssignment(success.entity.id));
        }, (fail) => {
            console.log(fail);
            dispatch({
                type: Actions.USER_ACCEPT_ASSIGNMENT_FAIL,
                payload: fail
            });
        });

    }
}

/**
 * loads the assignments that the server gives. Server is responsible to create new assignments if needed...
 *
 */
export function getActiveAssignment() {
    return (dispatch, getState) => {
        const store = getState();

        Rest.wrap(template)
            .wrap(timeout, {timeout: 10e6})
            .wrap(mime)
            .wrap(errorCode, {code: 500})({
                method: "GET",
                path: `/assignments/latest`,
                headers: {
                    "u": store.users.user.id,
                    "Content-Type": "application/json"
                }
            })
            .then((success) => {
                dispatch({
                    type: Actions.USER_LOAD_ACTIVE_ASSIGNMENT_SUCCESS,
                    payload: success.entity
                });
                dispatch(loadAssignmentTemplate(success.entity.assignmentTemplate));
            }, (fail) => {
                console.log(fail);
                dispatch({
                    type: Actions.USER_LOAD_ACTIVE_ASSIGNMENT_FAIL,
                    payload: fail
                });
            });

    }
}

export function saveFeedback(data = []) {
    return (dispatch, getState) => {
        const store = getState();

        const assignment = store.assignments.data.filter(e => e.active).shift();

        const assignmentId = assignment.id;
        const userId = store.users.user.id;

        dispatch({
            type: AssignmentsActions.PATCH_ASSIGNMENT_REQUEST
        });
        // dispatch({
        //     type: SummaryActionTypes.SUMMARY_LOAD_REQUEST
        // });

        Rest.wrap(template)
            .wrap(timeout, {timeout: 360*1000})
            .wrap(mime)
            .wrap(errorCode, {code: 500})({
                method: "PATCH",
                path: `/assignments/${assignmentId}`,
                headers: {
                    "u": userId,
                    "Content-Type": "application/json"
                },
                entity: {
                    items: data
                }
            })
            .then((success) => {
                dispatch({
                    type: AssignmentsActions.PATCH_ASSIGNMENT_SUCCESS,
                    payload: success.entity
                });

                dispatch(UserAssignmentsActions.loadAssignments());
                // dispatch({
                //     type: Actions.USER_LOAD_ACTIVE_ASSIGNMENT_SUCCESS,
                //     payload: success.entity
                // });
                // dispatch({
                //     type: SummaryActionTypes.RECOMMENDATION_LOAD_SUCCESS,
                //     payload: success.entity.interactions
                // });
                // dispatch({
                //     type: SummaryActionTypes.SUMMARY_LOAD_SUCCESS,
                //     entity: success.entity
                // });
            }, (fail) => {
                dispatch({
                    type: AssignmentsActions.PATCH_ASSIGNMENT_FAIL,
                    payload: fail
                });
            });
    }
}

/**
 * sets the active assignment and triggers a state refresh
 * @param assignmentId the assignment to set active
 */
export function activateAssignment(assignmentId) {
    return (dispatch, getState) => {
    const store = getState();

    const userId = store.users.user.id;

    dispatch({
        type: Actions.USER_LOAD_ACTIVE_ASSIGNMENT_REQUEST
    });

    Rest.wrap(template)
        .wrap(timeout)
        .wrap(mime)({
            method: "GET",
            path: `/assignments/${assignmentId}/activate`,
            headers: {
                "u": userId,
                "Content-Type": "application/json"
            }
        })
        .then((success) => {
            switch(success.status.code) {
                case 200:
                    dispatch(getActiveAssignment());
                    // dispatch({
                    //     type: Actions.USER_LOAD_ACTIVE_ASSIGNMENT_SUCCESS,
                    //     payload: success.entity
                    // });
                    // dispatch(loadAssignmentTemplate(success.entity.assignmentTemplate));
                    //
                    break;
                default:
                    dispatch({
                        type: Actions.USER_LOAD_ACTIVE_ASSIGNMENT_FAIL,
                        payload: success.entity
                    });
                    break;
            }
        }, (error) => {
            dispatch({
                type: Actions.USER_LOAD_ACTIVE_ASSIGNMENT_FAIL,
                payload: error.entity
            });
        });
    }
}
