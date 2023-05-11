const mariadb = require('mariadb');
let mwWert, mwZeit, mpEinheit, mpMass
const config = {
    host: '95.128.203.13',
    port: '3306',
    user: 'web204',
    password: 't0pUUqS9',
};
const pool = mariadb.createPool(config);
async function Read() {
    console.log("read")
    let conn;
    try {
        conn = await pool.getConnection();
        await conn.query('USE usr_web204_3');
        const rows = await conn.query('SELECT * FROM Messwerte');
        mwWert = await conn.query('SELECT mw_wert FROM Messwerte ORDER BY mw_id DESC LIMIT 1');
        return rows;
    } catch (err) {
        throw err;
    } finally {
        if (conn) conn.release();
    }
}
Read().then((rows) => {
    console.log(rows);
    console.log('Read succesful');
    console.log(mwWert);
    const win = window.open("uWeather.html")
    win.onload = function(){
        document.getElementById("#temp").innerHTML = mwWert
    }
}).catch((err) => {
    console.error(err);
});