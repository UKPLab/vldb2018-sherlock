import * as Actions from './constants';
import Rest from 'rest';
import template  from 'rest/interceptor/template';
import timeout from 'rest/interceptor/timeout';
import errorCode from 'rest/interceptor/errorCode';
import {loadAssignmentTemplate} from "../assignmenttemplates/actions";

export function loadSummary() {
    return (dispatch, getState) => {
        dispatch({
            type: Actions.SUMMARY_LOAD_REQUEST
        });

        const store = getState();

        let assignment_id = store.assignment.data.id;

        Rest.wrap(template)
            .wrap(timeout, {timeout: 10e6})
            .wrap(errorCode)({
                path: `/summary/assignment/${assignment_id}`,
                headers: {
                    "u": store.users.user.id
                }
            }).then((response) => {
            let entity = JSON.parse(response.entity);
            dispatch({
                type: Actions.SUMMARY_LOAD_SUCCESS,
                entity: entity,
            });
            dispatch(loadRecommendations());
            dispatch(loadAssignmentTemplate(entity.assignmentTemplate));
        }, (error) => {
            console.log("error");
            dispatch({
                type: Actions.SUMMARY_LOAD_FAIL,
                payload: error
            });
        });

    }
}

export function loadRecommendations() {
    return (dispatch, getState) => {
        dispatch({
            type: Actions.RECOMMENDATION_LOAD_REQUEST
        });

        const store = getState();

        let assignment_id = store.assignment.data.id;
        let user_id = store.users.user.id;
        let weights = store.summary.weights;

        Rest.wrap(template)
            .wrap(timeout, {timeout: 10e6})
            .wrap(errorCode)({
                method: "GET",
                path: `/summary/recommendation/${assignment_id}`,
                headers: {
                    "u": user_id
                }
            }).then((response) => {
            dispatch({
                type: Actions.RECOMMENDATION_LOAD_SUCCESS,
                payload: JSON.parse(response.entity),
            });
        }, (error) => {
            console.log("error");
            dispatch({
                type: Actions.RECOMMENDATION_LOAD_FAIL,
                payload: error
            });
        });


    }
}
