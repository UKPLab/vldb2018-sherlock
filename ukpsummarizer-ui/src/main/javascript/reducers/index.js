import {combineReducers} from 'redux';
import {routerReducer} from 'react-router-redux';
import datasets  from '../controllers/datasets/reducer';
import users from '../controllers/user/reducer';
import app from '../controllers/app/reducer';
import assignments from '../controllers/assignments/reducer';
import assignmentTemplates from '../controllers/assignmenttemplates/reducer';
import summary from '../controllers/summary/reducer';
export default combineReducers({
    datasets,
    users,
    app,
    summary,
    assignments,
    assignmentTemplates,
    routing: routerReducer
})
;
