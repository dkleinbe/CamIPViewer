
const express = require('express')
const router = express.Router()
const { check, validationResult } = require('express-validator/check')
const { matchedData } = require('express-validator/filter')
const fs = require('fs')

const content = 'Some content!'
settings = {}

router.get('/', (req, res) => {
  res.render('index')
})

router.get('/cameras', (req, res) => {
  
  let rawdata = fs.readFileSync('camera.json')
  settings = JSON.parse(rawdata)
  
  console.log(settings.cameras)

  //data = { "cam_ip": "192.1.2.3", "cam_port": "12345", "cameras": [ {"cam_ip": "192.1.2.66", "cam_port": "12345"}, {"cam_ip": "192.1.2.33", "cam_port": "12345"}] }
  var data = {}
  data.cameras = settings.cameras
  console.log(data)
  res.render('cameras', {
    data: data,
    errors: [],
    errorMap: {},
    csrfToken: req.csrfToken()
  })
})

router.post('/cameras', [
  check('cam_id')
    .isNumeric()
    .withMessage('Port should be a number')
    .trim(),
  check('cam_id')
    .isAlphanumeric()
    .withMessage('Not a valid camera Id')
    .trim(),
  check('cam_ip')
    //.isIP()
    //.withMessage('Not a valid IP')
    .trim(),
  check('cam_port')
    .isNumeric()
    .withMessage('Port should be a number')
    .trim()
], (req, res) => {
  const errors = validationResult(req)
  if (!errors.isEmpty()) {
    //console.log('req: ', req.body)
    return res.render('cameras', {
      data: req.body,
      errors: errors.array(),
      errorMap: errors.mapped(),
      csrfToken: req.csrfToken()
    })
  }
  //console.log('req: ', req.body)
  const data = matchedData(req)
  console.log('Sanitized: ', data)
  // Homework: send sanitized data in an email or persist in a db

  settings.cameras[data.cam_id] = data

  let jsonDta = JSON.stringify(settings);
  


  fs.writeFile('camera.json', jsonDta, err => {
    if (err) {
      console.error(err)
      return
    }
    //file written successfully
  })

  req.flash('success', 'Thanks for the message! I‘ll be in touch :)')
  res.redirect('/cameras')
})

router.get('/contact', (req, res) => {
  res.render('contact', {
    data: {},
    errors: [],
    errorMap: {},
    csrfToken: req.csrfToken()
  })
})

router.post('/contact', [
  check('message')
    .isLength({ min: 1 })
    .withMessage('Message is required :|')
    .trim(),
  check('email')
    .isEmail()
    .withMessage('That email doesn‘t look right')
    .trim()
    .normalizeEmail()
], (req, res) => {
  const errors = validationResult(req)
  if (!errors.isEmpty()) {
    return res.render('contact', {
      data: req.body,
      errors: errors.array(),
      errorMap: errors.mapped(),
      csrfToken: req.csrfToken()
    })
  }

  const data = matchedData(req)
  console.log('Sanitized: ', data)
  // Homework: send sanitized data in an email or persist in a db

  let jsonDta = JSON.stringify(data);
  fs.writeFile('camera.json', jsonDta, err => {
    if (err) {
      console.error(err)
      return
    }
    //file written successfully
  })

  req.flash('success', 'Thanks for the message! I‘ll be in touch :)')
  res.redirect('/')
})

module.exports = router
