import * as Actions from './constants';
import Rest from 'rest';
import template from 'rest/interceptor/template';
import timeout from 'rest/interceptor/timeout';
import errorCode from 'rest/interceptor/errorCode';
import mime from 'rest/interceptor/mime';

export function loadAssignmentTemplate(id) {
    return (dispatch, getState) => {
        if (id === undefined || id === null) {
            dispatch({
                type: Actions.LOAD_ASSIGNMENT_TEMPLATE_FAIL,
                payload: {
                    id: id,
                    error: "No id given"
                }
            })
        }


        const store = getState();

        if(store.assignmentTemplates.has(id)) {
            // we dont have to fetch data twice.
            return;
        }

        dispatch({
            type: Actions.LOAD_ASSIGNMENT_TEMPLATE_REQUEST,
            payload: {id: id}
        });

        let rest = Rest.getDefaultClient()
            .wrap(mime)
            .wrap(template)
            .wrap(errorCode, {code: 400})
            .wrap(errorCode, {code: 404})
            .wrap(errorCode, {code: 500});

        rest({
            method: "GET",
            path: `/assignmentTemplates/${id}`,
        }).then((success) => {
            const data = success.entity;

            dispatch({
                type: Actions.LOAD_ASSIGNMENT_TEMPLATE_SUCCESS,
                payload: {
                    id, data
                }
            });
        }, (error) => {
            dispatch({
                type: Actions.LOAD_ASSIGNMENT_TEMPLATE_FAIL,
                payload: {
                    error
                }
            })
        })
    }
}

export function loadAssignmentTemplates() {
    return (dispatch, getState) => {
        const store = getState();

        dispatch({
            type: Actions.LOAD_ASSIGNMENT_TEMPLATES_REQUEST
        });
        let rest = Rest.getDefaultClient()
            .wrap(mime)
            .wrap(template)
            .wrap(errorCode, {code: 400})
            .wrap(errorCode, {code: 404})
            .wrap(errorCode, {code: 500});

        rest({
            method: "GET",
            path: `/assignmentTemplates/forUser`,
            headers: {
                "u": store.users.user.id
            }
        }).then((success) => {
            const data = success.entity;
            dispatch({
                type: Actions.LOAD_ASSIGNMENT_TEMPLATES_SUCCESS,
                payload: data
            });
        }, (error) => {
            dispatch({
                type: Actions.LOAD_ASSIGNMENT_TEMPLATES_FAIL,
                payload: {
                    error
                }
            })
        });
    }
}
