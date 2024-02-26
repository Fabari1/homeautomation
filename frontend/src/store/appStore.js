import {defineStore} from 'pinia'
import {ref} from 'vue'


export const useAppStore =  defineStore('app', ()=>{

    /*  
    The composition API way of defining a Pinia store
    ref() s become state properties
    computed() s become getters
    function() s become actions  
    */ 

   const getUpdateData= async (data) => {
       const controller = new AbortController();
       const signal = controller.signal;
       const id= setTimeout(() => controller.abort(), 60000);
       const URL= '/api/update';

         try {
                const response = await fetch(URL, {
                    method: 'POST',
                    signal: signal,
                    headers: {
                        'Content-Type': 'application/json'
                    },
                    body: JSON.stringify(data)
                });
                const result = await response.json();
                console.log(result);
                clearTimeout(id);
                return result;
            } catch (error) {
                console.log(error);
                clearTimeout(id);
                return {status: 'getUpdatedata error', message: error.message};
            }
            return [];
    }
    
    const getSetPwd= async (data) => {
        const controller = new AbortController();
        const signal = controller.signal;
        const id= setTimeout(() => controller.abort(), 60000);
        const URL= '/api/set/combination';
        console.log(data);
        let text = data.toString();

             try {
                  const response = await fetch(URL, {
                        method: 'POST',
                        signal: signal,
                        body: JSON.stringify({"code": text}),
                        headers: {
                             'Content-Type': 'application/json'
                        }
                  });

                  console.log(response);
                  const result = await response.json();
                  console.log(result);
                  clearTimeout(id);
                  return result;
                 } catch (error) {
                  console.log(error);
                  clearTimeout(id);
                  return {status: 'getSetPwd error', message: error.message};
                 }
                 return [];

    }

    const getCheckPwd= async (data) => {
        const controller = new AbortController();
        const signal = controller.signal;
        const id= setTimeout(() => controller.abort(), 60000);
        const URL= '/api/check/combination';
            
             try {
                  const response = await fetch(URL, {
                        method: 'POST',
                        signal: signal,
                        headers: {
                             'Content-Type': 'application/json'
                        },
                        body: JSON.stringify(data)
                  });
                  const result = await response.json();
                  console.log(result);
                  clearTimeout(id);
                  return result;
                 } catch (error) {
                  console.log(error);
                  clearTimeout(id);
                  return {status: 'getCheckPwd error', message: error.message};
                 }
                 return [];

    }

    const getRetrieveData= async (start, end) => {
        const controller = new AbortController();
        const signal = controller.signal;
        const id= setTimeout(() => controller.abort(), 60000);
        const URL= `/api/reserve/${start}/${end}`;
        try {
            const response = await fetch(URL, {
                method: 'GET',
                signal: signal});
                if (response.ok) {
                    const data = await response.json();
                    console.log(data);

                    let keys = Object.keys(data);
                    if (keys.includes('status')) {
                        if(data["status"]=="complete"){
                            return data["data"];
                        }
                    if (data["status"]=="failed"){
                        console.log("getRetrieveData returned no data");
                    }
                }
            }
            else {
                const data = await response.text();
                console.log(data);
            }
        } catch (err) {
            console.log('getRetrieveData error', err.message);
        }
        return [];
    }

    const getCalculateAvg= async (start,end) => {
        const controller = new AbortController();
        const signal = controller.signal;
        const id = setTimeout(() => { controller.abort() }, 60000);
        const URL = `/api/avg/${start}/${end}`;
        try {
            const response = await fetch(URL, { method: 'GET', signal: signal });
            if (response.ok) {
                const data = await response.json();
                let keys = Object.keys(data);
                if (keys.includes("status")) {
                    if (data["status"] == "complete") {
                        console.log(data["data"]);
                        return data["data"];
                    }
                    if (data["status"] == "failed"
                    ) {
                        console.log("getCalculateAverage returned no data");
                    }
                }
            }
            else {
                const data = await response.text();
                console.log(data);
            }
        }
        catch (err) {

            console.error('getAllInRange error:', err.message);
        }
        return []
    }

        

    // STATES 

    // ACTIONS
 
   
 
 
    return { 
    // EXPORTS	
        getUpdateData,
        getSetPwd,
        getCheckPwd,
        getRetrieveData,
        getCalculateAvg    
       }
},{ persist: true  });